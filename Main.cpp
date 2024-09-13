#include <iostream>
#include <string>
#include "stack.h"
#include "maze.h"

#include "API.h"


Maze maze;
Heading m_heading;
Location m_location;
Location nextLocation;

#define TARGET1 Location(7,7)
#define TARGET2 Location(7,8)
#define TARGET3 Location(8,7)
#define TARGET4 Location(8,8)

void log(const std::string &text)
{
    std::cerr << text << std::endl;
}


Location get_next_unvisited_cell()
{
    for (int h = 0; h < HEADING_COUNT; h++)
    {
        Heading heading = static_cast<Heading>(h);
        nextLocation = m_location.neighbour(heading);
        if (maze.is_exit(m_location, heading) && !maze.cell_is_visited(nextLocation))
        {
            return nextLocation;
        }
    }
    return m_location; // No unvisited neighbors
}

Heading get_heading_to(Location next)
{
    if (next == m_location.north())
        return NORTH;
    if (next == m_location.east())
        return EAST;
    if (next == m_location.south())
        return SOUTH;
    if (next == m_location.west())
        return WEST;
    return m_heading; // Should never happen
}

void turn_to(Heading new_heading)
{
    int turn = new_heading - m_heading;
    if (turn == 1 || turn == -3)
    {
        API::turnRight();
        m_heading = right_from(m_heading);
    }
    else if (turn == -1 || turn == 3)
    {
        API::turnLeft();
        m_heading = left_from(m_heading);
    }
    else if (turn == 2 || turn == -2)
    {
        API::turnLeft();
        API::turnLeft();

        m_heading = behind_from(m_heading);
    }
}

void move_to(Location next)
{
    Heading new_heading = get_heading_to(next);

    turn_to(new_heading);
    API::moveForward();
    m_location = next;
}


void update_map()
    {
        bool leftWall = API::wallLeft();
        bool frontWall = API::wallFront();
        bool rightWall = API::wallRight();

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

int main(int argc, char *argv[])
{
    log("Starting...");
    maze.initialise();
    API::setColor(0, 0, 'G');
    API::setText(0, 0, "abc");

    // while (true) {
    //     if (!API::wallLeft()) {
    //         API::turnLeft();
    //     }
    //     while (API::wallFront()) {
    //         API::turnRight();
    //     }
    //     API::moveForward();
    // }

    maze.set_mask(MASK_OPEN);

    m_location = START;
    update_map();

    Stack<Location, 256> path;
    path.push(m_location);

    while (m_location != TARGET1 && m_location != TARGET2 && m_location != TARGET3 && m_location != TARGET4)
    {
        update_map();
        
        Location next_cell = get_next_unvisited_cell(); // If the next cell is unvisited, then this will return it. if there isn't this will return current location
        log("Moving from " + std::to_string(m_location.x) + "," + std::to_string(m_location.y) + " to " + std::to_string(next_cell.x) + "," + std::to_string(next_cell.y));
        if (next_cell != m_location)
        {   
            log("2");
            move_to(next_cell);
            path.push(next_cell);
            log("Moving from " + std::to_string(m_location.x) + "," + std::to_string(m_location.y) + " to " + std::to_string(next_cell.x) + "," + std::to_string(next_cell.y));
        }
        else if (!path.isEmpty()) // if path isn't empty, then this will remove the current location from the stack
        {
            // Backtrack
            log("Moving from " + std::to_string(m_location.x) + "," + std::to_string(m_location.y) + " to " + std::to_string(next_cell.x) + "," + std::to_string(next_cell.y));
            path.pop();
            if (!path.isEmpty())
            {
                move_to(path.peek());
                log("Moving from " + std::to_string(m_location.x) + "," + std::to_string(m_location.y) + " to " + std::to_string(next_cell.x) + "," + std::to_string(next_cell.y));
            }
        }
        else
        {
            // Explored everything, but didn't find the goal
            break;
        }
    }
}
