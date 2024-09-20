#include <iostream>
#include <string>
#include "stack.h"
#include "maze.h"
#include <vector>

#include "API.h"

Maze maze;
Heading m_heading;
Location m_location;
Location nextLocation;

struct Operation
{
    int move; // 1 for straights, 0 for rotations
    int value;

    Operation(int m, int c) : move(m), value(c) {}
};

std::vector<Operation> operations;

void log(const std::string &text)
{
    std::cerr << text << std::endl;
}

// Location get_next_unvisited_cell()
// {
//     for (int h = 0; h < HEADING_COUNT; h++)
//     {
//         Heading heading = static_cast<Heading>(h);
//         nextLocation = m_location.neighbour(heading);
//         if (maze.is_exit(m_location, heading) && !maze.cell_is_visited(nextLocation))
//         {
//             return nextLocation;
//         }
//     }
//     return m_location; // No unvisited neighbors
// }

// Heading get_heading_to(Location next)
// {
//     if (next == m_location.north())
//         return NORTH;
//     if (next == m_location.east())
//         return EAST;
//     if (next == m_location.south())
//         return SOUTH;
//     if (next == m_location.west())
//         return WEST;
//     return m_heading; // Should never happen
// }

// void turn_to(Heading new_heading)
// {
//     int turn = new_heading - m_heading;
//     if (turn == 1 || turn == -3)
//     {
//         API::turnRight();
//         m_heading = right_from(m_heading);
//     }
//     else if (turn == -1 || turn == 3)
//     {
//         API::turnLeft();
//         m_heading = left_from(m_heading);
//     }
//     else if (turn == 2 || turn == -2)
//     {
//         API::turnLeft();
//         API::turnLeft();

//         m_heading = behind_from(m_heading);
//     }
// }

// void move_to(Location next)
// {
//     Heading new_heading = get_heading_to(next);

//     turn_to(new_heading);
//     API::moveForward();
//     m_location = next;
// }

void update_map()
{
    bool leftWall = API::wallLeft();
    bool frontWall = API::wallFront();
    bool rightWall = API::wallRight();

    log("Left: " + std::string(leftWall ? "1" : "0") +
        " Front Wall: " + std::string(frontWall ? "1" : "0") +
        " Right: " + std::string(rightWall ? "1" : "0"));

    switch (m_heading)
    {
    case NORTH:
        maze.update_wall_state(m_location, NORTH, frontWall ? WALL : EXIT);
        maze.update_wall_state(m_location, EAST, rightWall ? WALL : EXIT);
        maze.update_wall_state(m_location, WEST, leftWall ? WALL : EXIT);
        break;
    case EAST:
        maze.update_wall_state(m_location, EAST, frontWall ? WALL : EXIT);
        maze.update_wall_state(m_location, SOUTH, rightWall ? WALL : EXIT);
        maze.update_wall_state(m_location, NORTH, leftWall ? WALL : EXIT);
        break;
    case SOUTH:
        maze.update_wall_state(m_location, SOUTH, frontWall ? WALL : EXIT);
        maze.update_wall_state(m_location, WEST, rightWall ? WALL : EXIT);
        maze.update_wall_state(m_location, EAST, leftWall ? WALL : EXIT);
        break;
    case WEST:
        maze.update_wall_state(m_location, WEST, frontWall ? WALL : EXIT);
        maze.update_wall_state(m_location, NORTH, rightWall ? WALL : EXIT);
        maze.update_wall_state(m_location, SOUTH, leftWall ? WALL : EXIT);
        break;
    default:
        // This is an error. We should handle it.
        break;
    }
}

std::string directionToString(Heading hd)
{
    switch (hd)
    {
    case NORTH:
        return "North";
    case EAST:
        return "East";
    case SOUTH:
        return "South";
    case WEST:
        return "West";
    default:
        return "BLOCKED";
    }
}


void turn_to_face(Heading newHeading)
{
    unsigned char hdgChange = (newHeading + HEADING_COUNT - m_heading) % HEADING_COUNT;
    switch (hdgChange)
    {
    case AHEAD:
        break;

    case RIGHT:
        API::turnRight();
        break;
    case BACK:
        API::turnRight();
        API::turnRight();
        break;
    case LEFT:
        API::turnLeft();
        break;
    }
    m_heading = newHeading;
}

void search_to(Location target)
{
    maze.flood(target);
    Heading best_direction = maze.heading_to_smallest(m_location, m_heading);
    turn_to_face(best_direction);

    while (m_location != target)
    {

        // for (int p = 0; p < MAZE_WIDTH; p++)
        // {
        //     for (int q = 0; q < MAZE_HEIGHT; q++)
        //     {
        //         API::setText(p, q, std::to_string(maze.get_cost(p, q)));
        //     }
        // }

        log("\nCell X : " + std::to_string(m_location.x) + "  Y :" + std::to_string(m_location.y));

        update_map();

        maze.flood(target);
        unsigned char newHeading = maze.heading_to_smallest(m_location, m_heading);

        log("New Heading : " + directionToString(static_cast<Heading>(newHeading)) + "  Current Heading : " + directionToString(static_cast<Heading>(m_heading)) + "\n");
        unsigned char hdgChange = (newHeading - m_heading) & 0x3;

        if (m_location != target)
        {
            switch (hdgChange)
            {
            // each of the following actions will finish with the
            // robot moving and at the sensing point ready for the
            // next loop iteration
            case AHEAD:
                API::moveForward();
                break;
            case RIGHT:
                API::turnRight();
                m_heading = right_from(m_heading);
                API::moveForward();
                break;
            case BACK:
                API::turnLeft();
                API::turnLeft();
                m_heading = behind_from(m_heading);
                API::moveForward();
                break;
            case LEFT:
                API::turnLeft();
                m_heading = left_from(m_heading);
                API::moveForward();
                break;
            }
            m_location = m_location.neighbour(static_cast<Heading>(newHeading));
        }
    }
}


void run_to(Location target)
{
    maze.set_mask(MASK_CLOSED);
    maze.flood(target);
    Heading best_direction = maze.heading_to_smallest(m_location, m_heading);
    turn_to_face(best_direction);

    int straightsCount = 0;

    while (m_location != target)
    {

        unsigned char newHeading = maze.heading_to_smallest(m_location, m_heading);
        unsigned char hdgChange = (newHeading - m_heading) & 0x3;
        if (m_location != target)
        {
            if (hdgChange == AHEAD)
            {
                straightsCount++;
            }
            else
            {
                operations.emplace_back(1, straightsCount);
                straightsCount = 1;

                if (hdgChange == RIGHT)
                {
                    m_heading = right_from(m_heading);
                    operations.emplace_back(0, -90);
                }
                else if (hdgChange == LEFT)
                {
                    m_heading = left_from(m_heading);
                    operations.emplace_back(0, 90);
                }
                else
                {
                    static int direction = 1;
                    direction *= -1;

                    m_heading = behind_from(m_heading);
                    operations.emplace_back(0, direction * 180);
                }
            }

            m_location = m_location.neighbour(static_cast<Heading>(newHeading));
        }
    }
    if(straightsCount != 1){
        operations.emplace_back(1,straightsCount);
    }else{
        operations.emplace_back(1, 1);
    }


    for (const auto &op : operations)
    {
        log("Move  :  " + std::to_string(op.move) + "  Value  :  "  + std::to_string(op.value));
        if(op.move == 1){
            API::moveForward(op.value);
        }else{
            if (op.value == 90){
                API::turnLeft();
            }else if(op.value == -90){
                API::turnRight();
            }else{
                API::turnLeft();
                API::turnLeft();
            }
        }

    }
    operations.clear();
}

void search_maze(){
    m_location = START;
    search_to(TARGET);
    turn_to_face(NORTH);
}

void search_come_back(){
    m_location = TARGET;
    search_to(START);
    turn_to_face(NORTH);
}

void run_maze(){
    m_location = START;
    run_to(TARGET);
    turn_to_face(NORTH);
}

void run_come_back(){
    m_location = TARGET;
    run_to(START);
    turn_to_face(NORTH);
}

int main(int argc, char *argv[])
{
    log("Starting...");
    maze.initialise();
    API::setColor(0, 0, 'G');
    API::setText(0, 0, "abc");

    search_maze();
    search_come_back();
    run_maze();
    run_come_back();

    // m_location = START;
    // m_heading = NORTH;
    // search_to(TARGET);

    // maze.flood(START);
    // Heading best_direction = maze.heading_to_smallest(m_location, m_heading);
    // turn_to_face(best_direction);
    // search_to(START);
    // turn_to_face(NORTH);

    // m_location = START;
    // run_to(TARGET);


    // m_location = TARGET;
    // maze.flood(START);
    // Heading direction = maze.heading_to_smallest(m_location, m_heading);
    // turn_to_face(direction);
    // run_to(START);

    // while (true) {
    //     if (!API::wallLeft()) {
    //         API::turnLeft();
    //     }
    //     while (API::wallFront()) {
    //         API::turnRight();
    //     }
    //     API::moveForward();
    // }

    // maze.set_mask(MASK_OPEN);

    // m_location = START;
    // update_map();

    // Stack<Location, 256> path;
    // path.push(m_location);

    // while (m_location != TARGET1 && m_location != TARGET2 && m_location != TARGET3 && m_location != TARGET4)
    // {
    //     update_map();

    //     Location next_cell = get_next_unvisited_cell(); // If the next cell is unvisited, then this will return it. if there isn't this will return current location
    //     log("Moving from " + std::to_string(m_location.x) + "," + std::to_string(m_location.y) + " to " + std::to_string(next_cell.x) + "," + std::to_string(next_cell.y));
    //     if (next_cell != m_location)
    //     {
    //         log("2");
    //         move_to(next_cell);
    //         path.push(next_cell);
    //         log("Moving from " + std::to_string(m_location.x) + "," + std::to_string(m_location.y) + " to " + std::to_string(next_cell.x) + "," + std::to_string(next_cell.y));
    //     }
    //     else if (!path.isEmpty()) // if path isn't empty, then this will remove the current location from the stack
    //     {
    //         // Backtrack
    //         log("Moving from " + std::to_string(m_location.x) + "," + std::to_string(m_location.y) + " to " + std::to_string(next_cell.x) + "," + std::to_string(next_cell.y));
    //         path.pop();
    //         if (!path.isEmpty())
    //         {
    //             move_to(path.peek());
    //             log("Moving from " + std::to_string(m_location.x) + "," + std::to_string(m_location.y) + " to " + std::to_string(next_cell.x) + "," + std::to_string(next_cell.y));
    //         }
    //     }
    //     else
    //     {
    //         // Explored everything, but didn't find the goal
    //         break;
    //     }
    // }
}
