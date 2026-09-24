// =====================================================================
// Лабораторная работа №1 — «Юнит на символьном поле»
// Консольная игра: юнит перемещается по символьной матрице по командам
// пользователя. Демонстрирует: абстрактный класс, наследование,
// полиморфизм (выбор класса юнита пользователем через базовый указатель).
//
// Компиляция:  g++ -std=c++17 -Wall -o game game.cpp
// Запуск:      ./game
//
// Символы поля:
//   '_'  — пустая клетка
//   '0'  — стена (перемещение невозможно)
//   '$'  — монетка (даёт очки, исчезает после сбора)
//   '*'  — ловушка (отнимает жизнь, гаснет после срабатывания)
//   '@'  — текущее положение юнита (рисуется поверх поля, в саму
//          матрицу не записывается)
//
// Команды игрока:
//   w / a / s / d — движение вверх/влево/вниз/вправо
//   q             — выход из игры
//
// Условия окончания игры:
//   Победа    — собраны все монетки на поле.
//   Поражение — здоровье юнита опустилось до 0.
// =====================================================================

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <limits>

// ---------------------------------------------------------------------
// Класс игрового поля — символьная матрица и операции над ней.
// ---------------------------------------------------------------------
class GameField {
public:
    GameField(std::vector<std::string> layout)
        : grid(std::move(layout)) {
        height = static_cast<int>(grid.size());
        width = height > 0 ? static_cast<int>(grid[0].size()) : 0;
        totalCoins = 0;
        for (const auto& row : grid)
            for (char c : row)
                if (c == '$') ++totalCoins;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    bool inBounds(int x, int y) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }

    // Символ клетки; клетка вне поля трактуется как стена.
    char at(int x, int y) const {
        if (!inBounds(x, y)) return '0';
        return grid[y][x];
    }

    bool isWall(int x, int y) const { return at(x, y) == '0'; }

    // "Собрать" содержимое клетки (монетку/ловушку) — клетка становится пустой.
    void clearCell(int x, int y) {
        if (inBounds(x, y)) grid[y][x] = '_';
    }

    int coinsRemaining() const { return coinsCollected < totalCoins ? totalCoins - coinsCollected : 0; }
    void registerCoinCollected() { ++coinsCollected; }
    bool allCoinsCollected() const { return coinsCollected >= totalCoins; }

    // Отрисовка поля с юнитом, отображённым символом '@'.
    void draw(int unitX, int unitY) const {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (x == unitX && y == unitY)
                    std::cout << '@';
                else
                    std::cout << grid[y][x];
            }
            std::cout << '\n';
        }
    }

private:
    std::vector<std::string> grid;
    int width;
    int height;
    int totalCoins;
    int coinsCollected = 0;
};

// ---------------------------------------------------------------------
// Абстрактный класс юнита. Общее состояние (позиция, здоровье, очки)
// и общая логика взаимодействия с клеткой вынесены сюда; сам способ
// перемещения (сколько клеток за ход, что делать при стене и т.п.)
// каждый наследник определяет по-своему через чисто виртуальный move().
// ---------------------------------------------------------------------
class Unit {
public:
    Unit(GameField& fieldRef, int startX, int startY, int startHealth)
        : field(fieldRef), x(startX), y(startY), health(startHealth), score(0) {}

    virtual ~Unit() = default;

    // Обработать команду перемещения ('w','a','s','d'). Возвращает false,
    // если команда не распознана как направление движения.
    virtual bool move(char command) = 0;

    virtual std::string getClassName() const = 0;

    int getX() const { return x; }
    int getY() const { return y; }
    int getHealth() const { return health; }
    int getScore() const { return score; }
    bool isAlive() const { return health > 0; }

protected:
    // Общая для всех юнитов реакция на содержимое клетки, в которую
    // юнит переместился. Наследники вызывают её из своей реализации move().
    void interactWithCell(int cellX, int cellY) {
        char c = field.at(cellX, cellY);
        if (c == '$') {
            score += 10;
            field.registerCoinCollected();
            field.clearCell(cellX, cellY);
            std::cout << "  [+] Монетка собрана! Очки: " << score << "\n";
        } else if (c == '*') {
            --health;
            field.clearCell(cellX, cellY);
            std::cout << "  [!] Ловушка сработала! Здоровье: " << health << "\n";
        }
    }

    static void direction(char command, int& dx, int& dy) {
        dx = 0; dy = 0;
        switch (command) {
            case 'w': dy = -1; break;
            case 's': dy = 1;  break;
            case 'a': dx = -1; break;
            case 'd': dx = 1;  break;
        }
    }

    GameField& field;
    int x, y;
    int health;
    int score;
};

// ---------------------------------------------------------------------
// Наследник №1 — «Пехотинец» (Walker).
// Перемещается ровно на одну клетку за команду; в стену не проходит,
// но и не тратит ход впустую — просто остаётся на месте.
// ---------------------------------------------------------------------
class Walker : public Unit {
public:
    Walker(GameField& fieldRef, int startX, int startY)
        : Unit(fieldRef, startX, startY, /*startHealth=*/3) {}

    std::string getClassName() const override { return "Пехотинец (Walker)"; }

    bool move(char command) override {
        int dx, dy;
        direction(command, dx, dy);
        if (dx == 0 && dy == 0) return false; // не команда движения

        int nx = x + dx;
        int ny = y + dy;
        if (field.isWall(nx, ny) || !field.inBounds(nx, ny)) {
            std::cout << "  [x] Впереди стена, шаг невозможен.\n";
            return true;
        }
        x = nx;
        y = ny;
        interactWithCell(x, y);
        return true;
    }
};

// ---------------------------------------------------------------------
// Наследник №2 — «Разведчик» (Runner).
// За одну команду пытается пройти сразу 2 клетки в выбранном направлении
// (быстрее пересекает поле), собирая/срабатывая ловушки на каждой
// пройденной клетке. Если на пути встречается стена — останавливается
// перед ней, не теряя оставшийся "запас хода".
// ---------------------------------------------------------------------
class Runner : public Unit {
public:
    Runner(GameField& fieldRef, int startX, int startY)
        : Unit(fieldRef, startX, startY, /*startHealth=*/2) {}

    std::string getClassName() const override { return "Разведчик (Runner)"; }

    bool move(char command) override {
        int dx, dy;
        direction(command, dx, dy);
        if (dx == 0 && dy == 0) return false;

        const int stepsPerCommand = 2;
        for (int step = 0; step < stepsPerCommand; ++step) {
            int nx = x + dx;
            int ny = y + dy;
            if (field.isWall(nx, ny) || !field.inBounds(nx, ny)) {
                std::cout << "  [x] Стена остановила рывок после " << step << " клетки(ок).\n";
                break;
            }
            x = nx;
            y = ny;
            interactWithCell(x, y);
            if (!isAlive()) break;
        }
        return true;
    }
};

// ---------------------------------------------------------------------
// Вспомогательные функции игрового цикла.
// ---------------------------------------------------------------------
std::unique_ptr<Unit> chooseUnit(GameField& field, int startX, int startY) {
    std::cout << "Выберите класс юнита:\n"
              << "  1 - Пехотинец (обычный шаг, 3 жизни)\n"
              << "  2 - Разведчик (рывок на 2 клетки, 2 жизни)\n"
              << "Ваш выбор: ";

    int choice = 0;
    while (!(std::cin >> choice) || (choice != 1 && choice != 2)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Введите 1 или 2: ";
    }

    // Полиморфизм: обе ветки возвращаются через указатель на базовый
    // класс Unit, дальнейший код работает с юнитом одинаково независимо
    // от того, какой конкретный класс был выбран.
    if (choice == 1)
        return std::make_unique<Walker>(field, startX, startY);
    else
        return std::make_unique<Runner>(field, startX, startY);
}

void printStatus(const Unit& unit, const GameField& field) {
    std::cout << "Класс: " << unit.getClassName()
              << " | Здоровье: " << unit.getHealth()
              << " | Очки: " << unit.getScore()
              << " | Монет осталось: " << field.coinsRemaining() << "\n";
}

int main() {
    // Пример карты. '0' - стены, '$' - монетки, '*' - ловушки, '_' - пусто.
    std::vector<std::string> layout = {
        "0000000000",
        "0__$__*__0",
        "0_00_00_$0",
        "0__*____00",
        "0_0___0__0",
        "0$__*__$_0",
        "0000000000"
    };

    GameField field(layout);
    std::unique_ptr<Unit> unit = chooseUnit(field, /*startX=*/1, /*startY=*/1);

    std::cout << "\nУправление: w/a/s/d — движение, q — выход.\n\n";

    while (true) {
        field.draw(unit->getX(), unit->getY());
        printStatus(*unit, field);

        if (field.allCoinsCollected()) {
            std::cout << "\n*** ПОБЕДА! Все монетки собраны. Итоговый счёт: "
                      << unit->getScore() << " ***\n";
            break;
        }
        if (!unit->isAlive()) {
            std::cout << "\n*** ПОРАЖЕНИЕ! Здоровье закончилось. Итоговый счёт: "
                      << unit->getScore() << " ***\n";
            break;
        }

        std::cout << "Команда (w/a/s/d/q): ";
        char command;
        if (!(std::cin >> command)) break; // конец ввода (например, EOF)

        if (command == 'q') {
            std::cout << "Выход из игры.\n";
            break;
        }

        if (!unit->move(command)) {
            std::cout << "  [?] Неизвестная команда.\n";
        }
        std::cout << "\n";
    }

    return 0;
}