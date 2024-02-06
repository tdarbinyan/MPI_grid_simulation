# MPI_grid_simulation

Полное описание работы программы находится в самом коде в виде комментариев

## Запуск

В директории с файлом main.cpp прописать
```mpic++ main.cpp```
а дальше запустить командой
```mpirun -n 25 ./a.out ```

## Кастомизация
Для симуляции другой размерности сети нужно подобрать число, равное длине стороны матрицы из компьютеров, и в 15-ой строчке программы прописать его как значение ```SIZE```
При запуске в таком случае вместо 25 прописать квадрат выбранного числа

## Задача

В транспьютерной матрице размером 5x5, в каждом узле которой находится один процесс, необходимо переслать очень длинное сообщение (длиной L байт) из узла с координатами (0,0) в узел с координатами (4,4). Реализовать программу, моделирующую выполнение такой пересылки на транспьютерной матрице с использованием буферизуемого режима передачи сообщений MPI. Получить временную оценку работы алгоритма, если время старта равно 100, время передачи байта равно 1 (Ts=100, Tb=1). Процессорные операции, включая чтение из памяти и запись в память, считаются бесконечно быстрыми.

## Решение

При спуске из верхнего левого угла в правый нижний в матрице 5x5 переходами исключительно вправо или вниз потребуется 8 переходов, например по сторонам квадрата это будут следующие переходы:

```
(0, 0) -> (0, 1) -> (0, 2) -> (0, 3) -> (0, 4) -> (1, 4) -> (2, 4) -> (3, 4) -> (4, 4)
```

Получаем временную оценку `8 * (Ts + Tb * L)`. Однако ситуацию можно улучшить, пустив информацию по двум каналам, получив оценку `8 * (Ts + Tb * L / 2)`. На более чем 2 канала распараллелить поток информации нельзя, потому что узкими местами являются начальная и принимающая вершина, у которых соответственно по 2 канала.

## Ответ

Временная оценка работы алгоритма: `8 * (Ts + Tb * L / 2)`, где Ts = 100, Tb = 1.
