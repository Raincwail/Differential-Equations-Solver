# Differential-Equations-Solver
[Более подробное описание задачи](https://github.com/Raincwail/Differential-Equations-Solver/blob/master/Complete_Task.pdf)

## 1. Решение системы обыновенных дифференциальных уравнений
Дано $N$ материальных точек с массами $m_{k}$, положения которых в
начальный момент времени заданы радиус-векторами $r_{k}$, а скорости векторами $v_{k}$, $k = \overline{1, N}$.
Требуется определить траектории всех частиц во все моменты времени от 0 до $t_{end}$.

### Формат ввода
На вход программе дается файл с массами, начальными координатами и скоростями всех материальных точек.
Первая строка файла состоит из одного числа $n$ — количества точек.

### Формат вывода
Файл формата csv, каждая строка которого имеет следующий вид:
<p align="center">
$t$ $x1$ $y1$ $x2$ $y2$ $. . .$ $xn$ $yn$
</p>
