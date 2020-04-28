/**
    System specifications:
        clang++ info:
            Apple LLVM version 9.0.0 (clang-900.0.38)
            Target: x86_64-apple-darwin17.4.0
            Thread model: posix

        operating system: 10.13.03 macOS High Sierra

    filename: GameOfLife.cpp
    Purpose: Runnning this program will enable the user to play John Conway's
             game of life. The user will be able to simulate a sequence of timesteps,
             set many parameters, load in prefabricated grid patterns,
             and move around the grid.
*/


#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sstream>

using namespace std;

// Declare classes.
class Rand;
class Grid;
class Menu;

// Clears a terminal screen by printing 40 newlines.
void clear_screen() {
    for(int i = 0; i < 4; i++)
        cout << "\n\n\n\n\n\n\n\n\n\n";
}


/******** Rand Class ***********/
// A pseudorandom number generator that uses the
// Xorshift generator algorithm.
// The theoretical working of the algorithm
// can be found here https://goo.gl/5fNHYD
// and the specific implementation and parameter
// choice used in this class can be found
// here https://goo.gl/SmbrkY.
// If a Rand object is not initialised with a
// seed, the default seed of value 1 will be used.
// One must note that it is not possible to use
// To generate a random integer use rand_int().
class Rand {
    public:
        unsigned long  rand_long();
        Rand();
        Rand(unsigned long given_seed);

    private:
        unsigned long seed;
        unsigned long current_state;
};

Rand::Rand() {
    seed = 0;
    current_state = seed;
}

Rand::Rand(unsigned long given_seed) {
    if (given_seed == 0) {
        cerr << "One can not use the value 0 as a "
             << "seed for an Xorshift generator." << endl;
        given_seed = 1;
    }
    seed = given_seed;
    current_state = seed;
}

unsigned long Rand::rand_long() {
    current_state ^= (current_state << 21);
    current_state ^= (current_state >> 35);
    current_state ^= (current_state << 4);
    return current_state;
}


/******** Grid Class ***********/
// A grid that keeps track of the state of
// the game of life 200x200 grid which
// contains only lifing and dead cells.
class Grid {
    public:
        Grid(Rand *passed_rand);
        void display_grid();
        void move_origin(int change_x, int change_y);
        void set_prob(double prob);
        void set_live_char(char live);
        void set_dead_char(char dead);
        void set_hor_step_size(int step_size);
        void set_vert_step_size(int step_size);
        void file_to_grid(string file_name);
        void randomize_grid();
        void clean_grid();
        void next_state();

    private:
        Rand *rand;
        int const static display_length = 40;
        int const static display_width = 80;
        int const static grid_length = 200;
        int const static grid_width = 200;
        int hor_step_size;
        int vert_step_size;
        int top_left_x;
        int top_left_y;
        double cell_prob;
        char live_char;
        char dead_char;
        bool grid[grid_length][grid_width];

        bool next_state_cell(int y, int x);
        bool in_range(int y, int x);
};

// Constructor
Grid::Grid(Rand *passed_rand) {
    hor_step_size = 1;
    vert_step_size = 1;
    top_left_x = 0;
    top_left_y = 0;
    cell_prob = 0.5;
    live_char = 'O';
    dead_char = '.';
    rand = passed_rand;
    clean_grid();
}

// Changes the origin of the canvas that is drawn to
// screen. This creates the impression of movement.
void Grid::move_origin(int change_x, int change_y) {
    int max_x = grid_width - display_width - 1;
    int max_y = grid_length - display_length - 1;

    top_left_x = top_left_x + (change_x * hor_step_size);
    top_left_y = top_left_y + (change_y * vert_step_size);

    if (top_left_x > max_x)
        top_left_x = max_x;
    else if (top_left_x < 0)
        top_left_x = 0;

    if (top_left_y > max_y)
        top_left_y = max_y;
    else if (top_left_y < 0)
        top_left_y = 0;
}

// Determines the state of a cell at the next
// time iteration. This is done according to the
// following rules
//  * A live cell with less than 2 live neighbors dies
//  * A live cell with 2 or 3 live neighbors lives on
//  * A live cell with more than 3 live neighbors dies
//  * A dead cell with 3 live neighbors becomes alive
bool Grid::next_state_cell(int y, int x) {
    int live_neighbors = 0;
    bool currently_alive = grid[y][x];

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (!(i == 0 && j == 0) && in_range(y+i,x+j) && grid[y+i][x+j])
                live_neighbors++;
        }
    }

    if (currently_alive) {
        if (live_neighbors < 2) {
            return false;
        } else if (live_neighbors > 3) {
            return false;
        } else {
            return true;
        }
    } else {
        if (live_neighbors == 3) {
            return true;
        } else {
            return false;
        }
    }

}

// Updates all the cells in a grid by fetching
// each cell's state for the next time step and assigning
// that state to each cell.
void Grid::next_state() {
    bool new_grid[grid_width][grid_length];

    for (int y = 0; y < grid_width; y++) {
        for (int x = 0; x < grid_length; x++) {
            bool new_state = next_state_cell(y, x);
            new_grid[y][x] = new_state;
        }
    }

    for (int y = 0; y < grid_width; y++) {
        for (int x = 0; x < grid_length; x++) {
            grid[y][x] = new_grid[y][x];
        }
    }

}

// Randomly initialises cells by calling the
// Rand class, setting distribution limits based
// on the variable 'cell_prob'.
void Grid::randomize_grid() {
    for(int i = 0; i < grid_length; i++) {
        for (int j = 0; j < grid_width; j++) {

            double prob = (ULONG_MAX - rand->rand_long()) / static_cast<double>ULONG_MAX;
            if (prob <= cell_prob)
                grid[i][j] = true;
            else
                grid[i][j] = false;
        }
    }
}

// Sets all the cells in the grid to 'dead'.
void Grid::clean_grid() {
    for(int i = 0; i < grid_length; i++)
        for(int j = 0; j < grid_width; j++)
            grid[i][j] = false;
}

// Checks whether a coordinate falls within the grid.
bool Grid::in_range(int y, int x) {
    if ((x < 0) || (x > grid_width) ||
        (y < 0) || (y > grid_length))
        return false;
    else
        return true;
}

// Sets the vertical step size with which
// a user moves over the grid.
void Grid::set_vert_step_size(int step_size) {
    if(step_size < grid_length) {
        vert_step_size = step_size;
    } else {
        cerr << "Please choose a value between "
             << "0 and 200 as a step size." << endl;
        sleep(2);
    }
}

// Sets the horizontal step size with which
// a user moves over the grid.
void Grid::set_hor_step_size(int step_size) {
    if(step_size < grid_width) {
        hor_step_size = step_size;
    } else {
        cerr << "Please choose a value between "
             << "0 and 200 as a step size." << endl;
        sleep(2);
    }
}

// Sets the probability with which a cell is
// alive when random initialisation is called.
void Grid::set_prob(double prob) {
    if ((prob >= 0) && (prob <= 1))
        cell_prob = prob;
    else {
        cerr << "Please choose a decimal value between "
             << "0 and 1 as a probability." << endl;
        sleep(2);
    }
}

// Sets the character which will be used to
// depict alive cells on the grid.
void Grid::set_live_char(char live) {
    live_char = live;
}

// Sets the character which will be used to
// depict dead cells on the grid.
void Grid::set_dead_char(char dead) {
    dead_char = dead;
}

// Displays the grid on the standard output.
void Grid::display_grid() {
    clear_screen();
    int y_max = top_left_y + display_length;
    int x_max = top_left_x + display_width;

    for(int y = top_left_y; y < y_max; y++) {
        for (int x = top_left_x; x < x_max; x++) {
            if (grid[y][x])
                cout << live_char;
            else
                cout << dead_char;
        }
        cout << endl;
    }
}

// Reads a grid pattern from a file.
void Grid::file_to_grid(string file_name) {
    ifstream input_file(file_name);

    if (!(input_file)) {
        cerr << "The file name " << file_name << " does not exists.";
    }

    string line;
    int char_count = 0;
    int line_count = 0;
    char current_character;

    while (getline(input_file, line)) {
        istringstream line_stream(line);
        char_count = 0;

        if (++line_count >= grid_length) {
            cerr << "The file contains too many lines.";
            clean_grid();
            return;
        }

        while (line_stream >> current_character) {
            if (++char_count >= grid_width) {
                cerr << "The file contains too many characters on a single line.";
                clean_grid();
                return;
            }

            if ((current_character == '.') || (current_character == ' ')) {
                grid[line_count-1][char_count-1] = false;
                cout << "false";
            } else {
                grid[line_count - 1][char_count - 1] = true;
                cout << "true";
            }
        }
    }
}


/******** Menu Class ***********/
// A menu that keeps track of which menu page
// the user is currently on. Any input given
// by a user is handled by this class and
// any changes of the underlying grid
// are passed to a Grid object.
class Menu {
    public:
        Menu(Grid *passed_grid);
        void handle_input(char input);
        void display_menu();

    private:
        // Current menu screen tracking variable
        short current_menu;

        // Access to the grid object which
        // the menu options affect.
        Grid *grid;

        // Display functions
        void display_main_menu();
        void display_move_menu();
        void display_param_menu();
        void display_file_menu();

        // Handle user input functions
        void handle_main_input(char lower_input);
        void handle_move_input(char lower_input);
        void handle_parameter_input(char lower_input);
        void handle_file_input(char lower_input);
};

// Constructor
Menu::Menu(Grid *passed_grid) {
    grid = passed_grid;
    current_menu = 0;
}

// The most abstract input handler.
void Menu::handle_input(char input) {

    // We will only pass lowercase letters as
    // user input to the menus.
    input = tolower(input);

    switch (current_menu) {
        case 0:
            handle_main_input(input);
            break;
        case 1:
            handle_move_input(input);
            break;
        case 2:
            handle_parameter_input(input);
            break;
        case 3:
            handle_file_input(input);
    }
}

// The input handler for any inputs given in the
// main menu.
void Menu::handle_main_input(char lower_input) {

    switch (lower_input) {
        case 'c':
            grid->clean_grid();
            break;
        case 'r':
            grid->randomize_grid();
            break;
        case 'o':
            grid->next_state();
            break;
        case 'g':
            for (int j = 0; j < 1000; j++) {
                usleep(50000);
                grid->next_state();
                grid->display_grid();
            }
            break;
        case 'm':
            current_menu = 1;
            break;
        case 'p':
            current_menu = 2;
            break;
        case 'f':
            current_menu = 3;
            break;
        case 'x':
            exit(0);
        default:
            grid->next_state();
            break;
    }
}

// The input handler for any inputs given in the
// move menu.
void Menu::handle_move_input(char lower_input) {
    switch (lower_input) {
        case 'w':
            grid->move_origin(0, -1);
            break;
        case 's':
            grid->move_origin(0, 1);
            break;
        case 'a':
            grid->move_origin(-1, 0);
            break;
        case 'd':
            grid->move_origin(1, 0);
            break;
        case 'm':
            current_menu = 0;
            break;
        default:
            break;
    }
}

// The input handler for any inputs given in the
// change parameter menu.
void Menu::handle_parameter_input(char lower_input) {
    switch (lower_input) {
        case 'h':
            cout << "\nEnter the desired horizontal step size: ";
            int hor_step_size;
            cin >> hor_step_size;
            grid->set_hor_step_size(hor_step_size);
            break;

        case 'v':
            cout << "\nEnter the desired verical step size: ";
            int vert_step_size;
            cin >> vert_step_size;
            grid->set_vert_step_size(vert_step_size);
            break;

        case 'p':
            cout << "\nEnter the desired cell alive probability: ";
            double prob;
            cin >> prob;
            grid->set_prob(prob);
            break;

        case 'l':
            cout << "\nEnter the desired alive cell character representation: ";
            char alive;
            cin >> alive;
            grid->set_live_char(alive);
            break;

        case 'd':
            cout << "\nEnter the desired dead cell character representation: ";
            char dead;
            cin >> dead;
            grid->set_dead_char(dead);
            break;

        case 'm':
            break;

        default:
            break;
    }
    current_menu = 0;
}

// The input handler for any inputs given in the
// file menu.
void Menu::handle_file_input(char lower_input) {
    string file_name;

    switch (lower_input) {
        case 'f':
            cout << "\nEnter the name of the file containing the data with which "
                 << "the grid will be filled: ";
            cin >> file_name;
            grid->file_to_grid(file_name);

            break;

        case 'm':
            break;

        default:
            break;
    }
    current_menu = 0;
}

// The most abstract display function.
void Menu::display_menu() {

    switch(current_menu) {
        case 0:
            display_main_menu();
            break;
        case 1:
            display_move_menu();
            break;
        case 2:
            display_param_menu();
            break;
        case 3:
            display_file_menu();
            break;
        default:
            display_main_menu();
            break;
    }
}

// Displays the main menu.
void Menu::display_main_menu() {
    cout << "[X] Stop \t [C] Clean \t [R] Randomize \t [O] One" << endl;
    cout << "[G] Go \t [M] Move \t [P] Parameter \t [F] File" << endl;
}

// Displays the menu in which a user can move.
void Menu::display_move_menu() {
    cout << "[W] Up \t [A] Left \t [S] Down \t [D] Right \t [M] Main Menu" << endl;
}

// Displays the menu in which a user can chance parameters.
void Menu::display_param_menu() {
    clear_screen();
    cout << "[H] Set horizontal step size. \n"
         << "[V] Set vertical step size Left. \n"
         << "[P] Set cell alive probability. \n"
         << "[L] Set alive cell character representation. \n"
         << "[D] Set dead cell character representation. \n"
         << "[M] Back to main menu. \n";
}

// Displays the menu in which a user can load a pattern file.
void Menu::display_file_menu() {
    cout << "[F] Choose a file to build a grid from. \n"
         << "[M] Back to main menu. \n";
}

int main() {
    Rand rand = Rand(4);
    Grid grid = Grid(&rand);
    Menu menu = Menu(&grid);

    char userInput;

    while (true) {
        grid.display_grid();
        menu.display_menu();
        cin >> userInput;
        menu.handle_input(userInput);
    }
}
