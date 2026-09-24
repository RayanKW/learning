// =====================================================================
// Lab Assignment #1 — "Unit on a symbol grid"
// A console game: a unit moves across a character matrix based on
// commands typed by the player. Demonstrates: an abstract class,
// inheritance, and polymorphism (the player picks a unit class, and
// the rest of the program only ever talks to it through the base
// class pointer).
//
// Build:  g++ -std=c++17 -Wall -o game game.cpp
// Run:    ./game
//
// Field symbols:
//   '_'  — empty cell
//   '0'  — wall (cannot move onto it)
//   '$'  — coin (adds score, disappears once collected)
//   '*'  — trap (removes one life, disappears once triggered)
//   '@'  — the unit's current position (drawn on top of the grid when
//          printing; it is never actually stored in the grid itself)
//
// Player commands:
//   w / a / s / d — move up / left / down / right
//   q             — quit the game
//
// End conditions:
//   WIN  — every coin on the field has been collected.
//   LOSE — the unit's health has dropped to 0.
// =====================================================================

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <limits>
#include <cctype>

// ---------------------------------------------------------------------
// GameField
// Wraps the character matrix and every operation performed on it:
// bounds checking, wall checking, "consuming" a coin/trap cell, and
// drawing the field (with the unit's position overlaid).
// ---------------------------------------------------------------------
class GameField {
public:
    explicit GameField(std::vector<std::string> layout)
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

    // Returns the symbol at (x, y). Anything outside the grid counts
    // as a wall, so callers never need a separate bounds check first.
    char at(int x, int y) const {
        if (!inBounds(x, y)) return '0';
        return grid[y][x];
    }

    bool isWall(int x, int y) const { return at(x, y) == '0'; }

    // "Consume" whatever was on this cell (coin or trap) — the cell
    // becomes empty afterwards, so it cannot trigger again.
    void clearCell(int x, int y) {
        if (inBounds(x, y)) grid[y][x] = '_';
    }

    int coinsRemaining() const {
        return coinsCollected < totalCoins ? totalCoins - coinsCollected : 0;
    }
    void registerCoinCollected() { ++coinsCollected; }
    bool allCoinsCollected() const { return coinsCollected >= totalCoins; }

    // Prints the field, drawing the unit as '@' at (unitX, unitY).
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
// Unit (abstract base class)
// Holds the state every unit needs (position, health, score) and the
// interaction logic shared by all of them. The actual *movement rule*
// — how many cells per command, what happens at a wall, etc. — is left
// to each subclass through the pure virtual move().
// ---------------------------------------------------------------------
class Unit {
public:
    Unit(GameField& fieldRef, int startX, int startY, int startHealth)
        : field(fieldRef), x(startX), y(startY), health(startHealth), score(0) {}

    virtual ~Unit() = default;

    // Handles one movement command ('w','a','s','d'). Returns false if
    // the character isn't a recognized direction at all.
    virtual bool move(char command) = 0;

    virtual std::string getClassName() const = 0;

    int getX() const { return x; }
    int getY() const { return y; }
    int getHealth() const { return health; }
    int getScore() const { return score; }
    bool isAlive() const { return health > 0; }

protected:
    // Shared reaction to whatever is on the cell the unit just stepped
    // onto. Every subclass calls this from inside its own move().
    void interactWithCell(int cellX, int cellY) {
        char c = field.at(cellX, cellY);
        if (c == '$') {
            score += 10;
            field.registerCoinCollected();
            field.clearCell(cellX, cellY);
            std::cout << "  [+] Coin collected! Score: " << score << "\n";
        } else if (c == '*') {
            --health;
            field.clearCell(cellX, cellY);
            std::cout << "  [!] Trap triggered! Health: " << health << "\n";
        }
    }

    // Converts a direction key into a (dx, dy) step. Returns (0, 0) for
    // anything that isn't w/a/s/d.
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
// Walker (Unit subclass #1)
// Moves exactly one cell per command. If a wall blocks the way, it
// simply stays put (the turn is not "wasted" on anything harmful).
// ---------------------------------------------------------------------
class Walker : public Unit {
public:
    Walker(GameField& fieldRef, int startX, int startY)
        : Unit(fieldRef, startX, startY, /*startHealth=*/3) {}

    std::string getClassName() const override { return "Walker (1 step, 3 lives)"; }

    bool move(char command) override {
        int dx, dy;
        direction(command, dx, dy);
        if (dx == 0 && dy == 0) return false; // not a movement key

        int nx = x + dx;
        int ny = y + dy;
        if (!field.inBounds(nx, ny) || field.isWall(nx, ny)) {
            std::cout << "  [x] A wall blocks the way — no step taken.\n";
            return true;
        }
        x = nx;
        y = ny;
        interactWithCell(x, y);
        return true;
    }
};

// ---------------------------------------------------------------------
// Runner (Unit subclass #2)
// Attempts to cover 2 cells per command instead of 1 (crosses the
// field faster), interacting with every cell it passes through. If a
// wall is hit mid-dash, it simply stops one cell early instead of
// carrying the remaining step over to the next command.
// ---------------------------------------------------------------------
class Runner : public Unit {
public:
    Runner(GameField& fieldRef, int startX, int startY)
        : Unit(fieldRef, startX, startY, /*startHealth=*/2) {}

    std::string getClassName() const override { return "Runner (2-cell dash, 2 lives)"; }

    bool move(char command) override {
        int dx, dy;
        direction(command, dx, dy);
        if (dx == 0 && dy == 0) return false;

        const int stepsPerCommand = 2;
        for (int step = 0; step < stepsPerCommand; ++step) {
            int nx = x + dx;
            int ny = y + dy;
            if (!field.inBounds(nx, ny) || field.isWall(nx, ny)) {
                std::cout << "  [x] The dash was stopped by a wall after "
                          << step << " cell(s).\n";
                break;
            }
            x = nx;
            y = ny;
            interactWithCell(x, y);
            if (!isAlive()) break; // no point continuing the dash if dead
        }
        return true;
    }
};

// ---------------------------------------------------------------------
// Input helpers
// ---------------------------------------------------------------------

// Reads one whole line and returns true with the trimmed line in
// `out`. Returns false only on end-of-input (e.g. Ctrl+D).
//
// NOTE ON WHY THIS USES getline INSTEAD OF `std::cin >> x`:
// `std::cin >> x` only reads a single token and leaves anything else
// typed on that line (extra letters, an accidental double key press,
// trailing spaces, a stray '\r' on Windows, etc.) sitting in the input
// buffer. On the *next* read, that leftover text is what gets consumed
// — not what the player just typed — which is exactly the kind of bug
// that can make the game seem to "quit by itself": a leftover 'q'
// hiding in the buffer from an earlier line gets read as if it were a
// fresh command. Reading a full line at a time and discarding anything
// unused avoids that class of bug entirely: every read starts with a
// clean, empty buffer.
bool readLine(const std::string& prompt, std::string& out) {
    std::cout << prompt;
    if (!std::getline(std::cin, out)) return false;
    // Trim a trailing '\r' (present if the input has Windows-style
    // line endings) and any surrounding whitespace.
    while (!out.empty() && std::isspace(static_cast<unsigned char>(out.back())))
        out.pop_back();
    size_t start = 0;
    while (start < out.size() && std::isspace(static_cast<unsigned char>(out[start])))
        ++start;
    out = out.substr(start);
    return true;
}

std::unique_ptr<Unit> chooseUnit(GameField& field, int startX, int startY) {
    std::cout << "Choose your unit class:\n"
              << "  1 - Walker  (regular 1-cell step, 3 lives)\n"
              << "  2 - Runner  (2-cell dash, 2 lives)\n";

    while (true) {
        std::string line;
        if (!readLine("Your choice: ", line)) {
            // No more input at all — default to Walker so the program
            // still has something valid to run with.
            return std::make_unique<Walker>(field, startX, startY);
        }
        if (line == "1")
            return std::make_unique<Walker>(field, startX, startY);
        if (line == "2")
            return std::make_unique<Runner>(field, startX, startY);
        std::cout << "Please enter 1 or 2.\n";
    }
}

void printStatus(const Unit& unit, const GameField& field) {
    std::cout << "Class: " << unit.getClassName()
              << " | Health: " << unit.getHealth()
              << " | Score: " << unit.getScore()
              << " | Coins left: " << field.coinsRemaining() << "\n";
}

int main() {
    // Sample map. '0' = wall, '$' = coin, '*' = trap, '_' = empty.
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

    std::cout << "\nControls: w/a/s/d = move, q = quit.\n\n";

    while (true) {
        field.draw(unit->getX(), unit->getY());
        printStatus(*unit, field);

        if (field.allCoinsCollected()) {
            std::cout << "\n*** YOU WIN! All coins collected. Final score: "
                      << unit->getScore() << " ***\n";
            break;
        }
        if (!unit->isAlive()) {
            std::cout << "\n*** GAME OVER! Health reached 0. Final score: "
                      << unit->getScore() << " ***\n";
            break;
        }

        std::string line;
        if (!readLine("Command (w/a/s/d/q): ", line)) {
            std::cout << "\nNo more input — exiting.\n";
            break; // end of input stream (e.g. Ctrl+D) — stop cleanly
        }

        if (line.empty()) {
            // The player just pressed Enter with nothing typed —
            // re-prompt instead of guessing what they meant.
            std::cout << "  [?] Empty command, please type w/a/s/d or q.\n\n";
            continue;
        }

        char command = static_cast<char>(std::tolower(static_cast<unsigned char>(line[0])));

        if (command == 'q') {
            std::cout << "Quitting the game.\n";
            break;
        }

        if (!unit->move(command)) {
            std::cout << "  [?] Unrecognized command: '" << command << "'.\n";
        }
        std::cout << "\n";
    }

    return 0;
}