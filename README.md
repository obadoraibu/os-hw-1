# os-hw-1
<br/> Завяьлов Егор Сергеевич БПИ-217 на 5 баллов<br/> Вариант 27

## 
Разработать программу, которая определяет частоту встречаемости (сколько раз встретилось в тексте) пяти ключевых
слов языка программирования C, в произвольной ASCII-строке.
Ключевые слова не должны являться частью идентификаторов.
Пять искомых ключевых слов выбрать по своему усмотрению.
Тестировать можно на файлах программ.

---- 
### Запуск:

Для тестирования и запуска программ необходимо,
находясь на самом высоком уровне, ввести в терминал:

> ./<>points/main ./4points/input<>.txt ./4points/output<>.txt </br>
# Отчёт 4 балла:

### 1. Приведено решение задачи на C:

> [main.c](https://github.com/obadoraibu/os-hw-1/blob/main/4%20points/main.c)
### 2. Запуск программы:

Компиляция:
> gcc main.c -o main <br>
### 3. Схема решаемой задачи:
Процессы:
> Первый: (дочерний главному): читает текстовые данные из заданного файла и через неименованный канал first_to_second
> передает их второму процессу. </br></br>
> Второй: (дочерний первому) осуществляет обработку(передает в функцию) полученных из канала данных и передает результат обработки через неименованный канал second_to_third третьему
> процессу. </br></br>
> Третий процесс: (дочерний второму) осуществляет вывод полученных данных из канала в
> файл </br>
<br>
### 4. Тесты проведены, удовлетворяют условию

# Отчёт 5 баллов:

### 1. Приведено решение задачи на C:

> [main.c](https://github.com/obadoraibu/os-hw-1/blob/main/5%20points/main.c)
### 2. Запуск программы:

Компиляция:
> gcc main.c -o main <br>
### 3. Схема решаемой задачи:
Процессы:
> Первый: (дочерний главному): читает текстовые данные из заданного файла и через именованный канал
> передает их второму процессу. </br></br>
> Второй: (дочерний первому) осуществляет обработку(передает в функцию) полученных из канала данных и передает результат обработки через именованный канал третьему
> процессу. </br></br>
> Третий процесс: (дочерний второму) осуществляет вывод полученных данных из канала в
> файл </br>
<br>
### 4. Тесты проведены, удовлетворяют условию
