#include <iostream>
#include <vector>
#include <string>

using namespace std;

// Game field class
class GameField {
public:
    GameField(vector<string> layout) {
        grid = layout;
        height = grid.size(); //number of rows
        width = grid[0].size(); //number of columns

        totalCoins = 0;
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++)
                if (grid[y][x] == '$') totalCoins++;
    }

    bool inBounds(int x, int y) {
        return x >= 0 && x < width && y >= 0 && y < height;
    }

    // Anything outside the grid also counts as a wall — so callers
    // never need a separate bounds check before calling isWall.
    bool isWall(int x, int y) {
        if (!inBounds(x, y)) return true;
        return grid[y][x] == '0';
    }

    char getSymbol(int x, int y) {
        if (!inBounds(x, y)) return '0';
        return grid[y][x];
    }

    // "Consume" a coin/trap — the cell becomes empty afterwards,
    // so it can't trigger a second time.
    void removeSymbol(int x, int y) {
        if (inBounds(x, y)) grid[y][x] = '_';
    }

    bool allCoinsCollected() { return coinsCollected >= totalCoins; }
    void coinCollected() { coinsCollected++; }
    int getCoinsLeft() { return totalCoins - coinsCollected; }

    void draw(int unitX, int unitY) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (x == unitX && y == unitY) cout << '@';
                else cout << grid[y][x];
            }
            cout << '\n';
        }
    }

private:
    vector<string> grid;
    int width, height;
    int totalCoins;
    int coinsCollected = 0;
};

// ---------- 2. Abstract unit class (requirement: "abstract class") ----------
class Unit {
public:
    Unit(GameField* f, int startX, int startY, int startHealth) {
        field = f;
        x = startX;
        y = startY;
        health = startHealth;
        score = 0;
    }

    virtual ~Unit() {}

    // "= 0" makes this a pure virtual function -> the class becomes
    // abstract, so a plain Unit object can never be created directly.
    virtual void move(char command) = 0;
    virtual string getName() = 0;

    int getX() { return x; }
    int getY() { return y; }
    int getHealth() { return health; }
    int getScore() { return score; }
    bool isAlive() { return health > 0; }

protected:
    // Shared reaction to whatever cell the unit just stepped onto —
    // every subclass calls this from inside its own move().
    void handleCell(int cx, int cy) {
        char c = field->getSymbol(cx, cy);
        if (c == '$') {
            score += 10;
            field->coinCollected();
            field->removeSymbol(cx, cy);
            cout << "  Coin collected! Score: " << score << "\n";
        } else if (c == '*') {
            health--;
            field->removeSymbol(cx, cy);
            cout << "  Trap triggered! Health: " << health << "\n";
        }
    }

    // Turns a w/a/s/d command into a (dx, dy) step.
    void getDirection(char command, int &dx, int &dy) {
        dx = 0; dy = 0;
        if (command == 'w') dy = -1;
        else if (command == 's') dy = 1;
        else if (command == 'a') dx = -1;
        else if (command == 'd') dx = 1;
    }

    GameField* field;
    int x, y;
    int health;
    int score;
};

//Subclass #1: moves one cell per command
class Walker : public Unit {
public:
    Walker(GameField* f, int startX, int startY) : Unit(f, startX, startY, 3) {}

    string getName() override { return "Walker (1 cell per move, 3 lives)"; }

    void move(char command) override {
        int dx, dy;
        getDirection(command, dx, dy);
        int nx = x + dx;
        int ny = y + dy;
        if (field->isWall(nx, ny)) {
            cout << "  A wall blocks the way — no step taken.\n";
            return;
        }
        x = nx;
        y = ny;
        handleCell(x, y);
    }
};

//Subclass #2 dashes two cells per command
class Runner : public Unit {
public:
    Runner(GameField* f, int startX, int startY) : Unit(f, startX, startY, 2) {}

    string getName() override { return "Runner (2-cell dash, 2 lives)"; }

    void move(char command) override {
        int dx, dy;
        getDirection(command, dx, dy);
        for (int step = 0; step < 2; step++) {
            int nx = x + dx;
            int ny = y + dy;
            if (field->isWall(nx, ny)) {
                cout << "  The dash was stopped by a wall.\n";
                break;
            }
            x = nx;
            y = ny;
            handleCell(x, y);
            if (!isAlive()) break;
        }
    }
};

// ---------- 4. Choosing a unit class (requirement: "polymorphism at selection") ----------
Unit* chooseUnit(GameField* field, int startX, int startY) {
    cout << "Choose your unit class:\n";
    cout << "  1 - Walker (1 cell per move, 3 lives)\n";
    cout << "  2 - Runner (2-cell dash, 2 lives)\n";

    int choice;
    while (true) {
        cout << "Your choice: ";
        cin >> choice;

        if (cin.fail()) {
            // The user typed something that isn't a number at all
            // (e.g. a letter). cin.fail() is now true, the stream is
            // "stuck", and the bad character is still waiting in the
            // buffer -- clear() resets the error flag, and ignore()
            // throws away everything up to the next newline so the
            // next read starts clean.
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "  Please enter a number (1 or 2).\n";
            continue;
        }

        if (choice == 1 || choice == 2) break;
        cout << "  Please enter 1 or 2.\n";
    }

    if (choice == 2)
        return new Runner(field, startX, startY);
    else
        return new Walker(field, startX, startY);
}

//Game loop (requirement: "loop: command -> unit -> redraw")
int main() {
    vector<string> layout = {
        "0000000000",
        "0__$__*__0",
        "0_00_00_$0",
        "0__*____00",
        "0_0___0__0",
        "0$__*__$_0",
        "0000000000"
    };

    GameField field(layout);
    Unit* unit = chooseUnit(&field, 1, 1); // <- unit's static type is Unit*,
                                            //    but it actually holds a
                                            //    Walker or a Runner —
                                            //    this IS the polymorphism

    cout << "\nControls: w/a/s/d = move, q = quit.\n\n";

    while (true) {
        field.draw(unit->getX(), unit->getY());
        cout << "Class: " << unit->getName()
             << " | Health: " << unit->getHealth()
             << " | Score: " << unit->getScore()
             << " | Coins left: " << field.getCoinsLeft() << "\n";

        if (field.allCoinsCollected()) {
            cout << "\n*** YOU WIN! Score: " << unit->getScore() << " ***\n";
            break;
        }
        if (!unit->isAlive()) {
            cout << "\n*** GAME OVER! Score: " << unit->getScore() << " ***\n";
            break;
        }

        cout << "Command (w/a/s/d/q): ";
        char command;
        cin >> command;

        if (command == 'q') {
            cout << "Quitting the game.\n";
            break;
        }

        unit->move(command); // <- same line of code calls either
                              //    Walker::move or Runner::move,
                              //    depending on what unit actually is
        cout << "\n";
    }

    delete unit; // freeing the memory that was allocated with `new` in chooseUnit
    return 0;
}