#include <iostream>
#include <mpi.h>

/* Схема перемещения данных
   0  >  1  > 2  > 3  > 4
   v                    v
   5     6    7    8    9
   v                    v
  10    11   12   13   14
   v                    v
  15    16   17   18   19
   v                    v
  20  > 21 > 22 > 23 > 24
*/
const int SIZE = 5;

class Data_t { // Класс для того, чтобы показать работу в 2 канала 
 public:
  int first_half; // Отправится через один канал(верхний)
  int second_half; // Отправится через другой(нижний)
}; // Это только примерный класс для удобства, на практике можно разделять любые данные

int neighbor_send(int rank) { // Получение номера соседа, которому нужно отправить данные
  if(rank < SIZE - 1 || rank >= SIZE * SIZE - SIZE) {
    return rank + 1;
  }
  if((rank + 1) % SIZE == 0 || rank % SIZE == 0) {
    return rank + SIZE;
  }
  return -1; // -1 Если нет соседа(вершина ни в одном из потоков, определенных по схеме выше)
}

int neighbor_recv(int rank) { // Получение номера соседа, с которого нужно принять данные
  if(rank < SIZE || rank > SIZE * SIZE - SIZE) {
    return rank - 1;
  }
  if((rank + 1) % SIZE == 0 || rank % SIZE == 0) {
    return rank - SIZE;
  }
  return -1; // Если нет соседа(вершина ни в одном из потоков, определенных по схеме выше)
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    // Получение номера нынешнего процесса и количества процессов
    int rank, size; 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // При запуске вводить количество процессов SIZE * SIZE (в данном случае 25)
    if (size != SIZE * SIZE) {
        std::cerr << "Number of processes must be equal to " << SIZE * SIZE << ", currently is " << size << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Симуляция поведения транспьюртерной сети
    if (rank == 0) { // Случай первой(стартовой) вершины
        Data_t data;
        data.first_half = 111;
        data.second_half = 222; // Любые данные, которые мы разделили и хотим отправить
        std::cout << "Starting data: " << data.first_half << " " << data.second_half << std::endl;

        int buffer_size = MPI_BSEND_OVERHEAD + sizeof(int);
        MPI_Buffer_attach(malloc(buffer_size), buffer_size);

        MPI_Bsend(&data.first_half, 1, MPI_INT, 1, 0, MPI_COMM_WORLD); // Отправили первую половину через верхний поток данных
        std::cout << "Process " << rank << " sent data to process " << 1 << std::endl;

        MPI_Bsend(&data.second_half, 1, MPI_INT, SIZE, 0, MPI_COMM_WORLD); // Отправили вторую половину через нижний поток данных
        std::cout << "Process " << rank << " sent data to process " << SIZE << std::endl;

        MPI_Buffer_detach(&data, &buffer_size);

    } else if (rank == SIZE * SIZE - 1) { // Случай последней(принимающей) вершины
        Data_t final_data;

        // Принимаем данные с верхней вершины(19 в данном случае)
        MPI_Recv(&final_data.first_half, 1, MPI_INT, SIZE * SIZE - SIZE - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process " << rank << " received data from process " << SIZE * SIZE - SIZE - 1 << std::endl;

        // Принимаем данные с левой вершины(23 в данном случае)
        MPI_Recv(&final_data.second_half, 1, MPI_INT, SIZE * SIZE - 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process " << rank << " received data from process " << SIZE * SIZE - 2 << std::endl;

        // Выводим полученные данные
        std::cout << "Final data: " << final_data.first_half << " " << final_data.second_half << std::endl;
    } else { // Случай всех остальных вершин
        
        int data;
        int sender = neighbor_recv(rank);
        if(sender != -1) { // Условие позволяет принять/отправить данные только если нынешняя вершина является частью потока данных(об этом говорилось выше)
          // Получение данных от предыдущего соседа
          MPI_Recv(&data, 1, MPI_INT, sender, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          std::cout << "Process " << rank << " received data from process " << sender << std::endl;

          // Отправка полученных данных к следующему соседу
          int receiver = neighbor_send(rank);
          int buffer_size = MPI_BSEND_OVERHEAD + sizeof(int);
          MPI_Buffer_attach(malloc(buffer_size), buffer_size);

          MPI_Bsend(&data, 1, MPI_INT, receiver, 0, MPI_COMM_WORLD);
          std::cout << "Process " << rank << " sent data to process " << receiver << std::endl;

          MPI_Buffer_detach(&data, &buffer_size);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}
