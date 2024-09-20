#ifndef MAZE_H
#define MAZE_H

#include <stdint.h>
#include <iostream>
#include <string>
#include "queue.h"
#include "API.h"

const int MAZE_WIDTH = 16;
const int MAZE_HEIGHT = 16;
const int MAZE_CELL_COUNT = MAZE_WIDTH * MAZE_HEIGHT;
const int MAX_COST = MAZE_CELL_COUNT - 1;
const int QUEUE_SIZE = 512;

#define START Location(0, 0)
#define TARGET Location(7, 7)

enum WallState
{
    EXIT = 0,    // a wall that has been seen and confirmed absent
    WALL = 1,    // a wall that has been seen and confirmed present
    UNKNOWN = 2, // a wall that has not yet been seen
    VIRTUAL = 3, // a wall that has not yet been seen
};

// 2 bits are reserved for each side. the 2 bit wallState will be stored there
struct WallInfo
{
    WallState north : 2;
    WallState east : 2;
    WallState south : 2;
    WallState west : 2;
};

enum MazeMask
{
    MASK_OPEN = 0x01,   // open maze for search
    MASK_CLOSED = 0x03, // closed maze for fast run
};

// functions for getting the heading from particular direction.
// for example if we turn right, then the new north for the robot will be
// east. so ifthe robot wants to get correct directions after turning right, it have to calculate the new
// heading using right_from(heading). this will return for relative north to be  absoulute south
enum Heading
{
    NORTH,
    EAST,
    SOUTH,
    WEST,
    HEADING_COUNT,
    BLOCKED = 99
};

inline Heading right_from(const Heading heading)
{
    return static_cast<Heading>((heading + 1) % HEADING_COUNT);
}

inline Heading left_from(const Heading heading)
{
    return static_cast<Heading>((heading + HEADING_COUNT - 1) % HEADING_COUNT);
}

inline Heading ahead_from(const Heading heading)
{
    return heading;
}

inline Heading behind_from(const Heading heading)
{
    return static_cast<Heading>((heading + 2) % HEADING_COUNT);
}

enum Direction
{
    AHEAD,
    RIGHT,
    BACK,
    LEFT,
    DIRECTION_COUNT
};

class Location
{
public:
    uint8_t x;
    uint8_t y;

    // using member initializer lists are said to be faster ?
    Location() : x(0), y(0){}; // default constructor will set the location to (0,0)
    Location(uint8_t ix, uint8_t iy) : x(ix), y(iy){};

    bool is_in_maze()
    {
        return x < MAZE_WIDTH && y < MAZE_HEIGHT;
    }

    // these operator overload functions will return, false or true when two location
    // objects are compared using != , == .
    bool operator==(const Location &obj) const
    {
        return x == obj.x && y == obj.y;
    }

    bool operator!=(const Location &obj) const
    {
        return x != obj.x || y != obj.y;
    }

    // these operators prevent the user from exceeding the bounds of the maze
    // by wrapping to the opposite edge
    Location north() const
    {
        return Location(x, (y + 1) % MAZE_HEIGHT);
    }

    Location east() const
    {
        return Location((x + 1) % MAZE_WIDTH, y);
    }

    Location south() const
    {
        return Location(x, (y + MAZE_HEIGHT - 1) % MAZE_HEIGHT);
    }

    Location west() const
    {
        return Location((x + MAZE_WIDTH - 1) % MAZE_WIDTH, y);
    }

    Location neighbour(const Heading heading) const
    {
        switch (heading)
        {
        case NORTH:
            return north();
            break;
        case EAST:
            return east();
            break;
        case SOUTH:
            return south();
            break;
        case WEST:
            return west();
            break;
        default:
            return *this; // this is actually an error and should be handled
            break;
        }
    }
};

class Maze
{
public:
    Maze()
    {
    }

    uint8_t get_cost(int x, int y)
    {
        return m_cost[x][y];
    }

    // returns wall info struct with 8 bits
    WallInfo walls(const Location cell) const
    {
        return m_walls[cell.x][cell.y];
    }

    // checks whther value in every side in the struct is equal to 10
    bool has_unknown_walls(const Location cell) const
    {
        WallInfo walls_here = m_walls[cell.x][cell.y];
        if (walls_here.north == UNKNOWN || walls_here.east == UNKNOWN || walls_here.south == UNKNOWN || walls_here.west == UNKNOWN)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool cell_is_visited(const Location cell) const
    {
        return not has_unknown_walls(cell);
    }

    // if wall is known then whether it is mask open or close it will return 00 ( ex : 00 & 01 or 00 & 11 -> exit , 01 & 01 or 01 & 11 will -> wall)
    //  if it is unknown then wallState is 10. so it will only return as a EXIT (00) if the mask is open (10 & 01 -> exit, 10 & 11 -> 10 (unknown))
    bool is_exit(const Location cell, const Heading heading) const
    {
        bool result = false;
        WallInfo walls = m_walls[cell.x][cell.y];

        switch (heading)
        {
        case NORTH:
            result = (walls.north & m_mask) == EXIT;
            break;
        case EAST:
            result = (walls.east & m_mask) == EXIT;
            break;
        case SOUTH:
            result = (walls.south & m_mask) == EXIT;
            break;
        case WEST:
            result = (walls.west & m_mask) == EXIT;
            break;
        default:
            result = false;
            break;
        }
        return result;
    }

    // only updates the wall if the given cell wall is unknown
    void update_wall_state(const Location cell, const Heading heading, const WallState state)
    {
        switch (heading)
        {
        case NORTH:
            if ((m_walls[cell.x][cell.y].north & UNKNOWN) != UNKNOWN)
            {
                return;
            }
            break;
        case EAST:
            if ((m_walls[cell.x][cell.y].east & UNKNOWN) != UNKNOWN)
            {
                return;
            }
            break;
        case WEST:
            if ((m_walls[cell.x][cell.y].west & UNKNOWN) != UNKNOWN)
            {
                return;
            }
            break;
        case SOUTH:
            if ((m_walls[cell.x][cell.y].south & UNKNOWN) != UNKNOWN)
            {
                return;
            }
            break;
        default:
            // ignore any other heading (blocked)
            break;
        }
        set_wall_state(cell, heading, state);
    }

    void initialise()
    {
        // initialize every wall to be unknown (10)
        for (int x = 0; x < MAZE_WIDTH; x++)
        {
            for (int y = 0; y < MAZE_HEIGHT; y++)
            {
                m_walls[x][y].north = UNKNOWN;
                m_walls[x][y].east = UNKNOWN;
                m_walls[x][y].south = UNKNOWN;
                m_walls[x][y].west = UNKNOWN;
            }
        }

        // setting corner most north south east west walls as WALL (01)
        for (int x = 0; x < MAZE_WIDTH; x++)
        {
            m_walls[x][0].south = WALL;
            m_walls[x][MAZE_HEIGHT - 1].north = WALL;
        }
        for (int y = 0; y < MAZE_HEIGHT; y++)
        {
            m_walls[0][y].west = WALL;
            m_walls[MAZE_WIDTH - 1][y].east = WALL;
        }

        // setting 0,0 wall states
        //   ―  for logging
        //  set_wall_state(START, EAST, WALL); //not sure whether this is necessary?
        //  set_wall_state(START, NORTH, EXIT);

        // the open maze treats unknowns as exits
        set_mask(MASK_OPEN);
        flood(TARGET);
    }

    void set_mask(const MazeMask mask)
    {
        m_mask = mask;
    }

    MazeMask get_mask() const
    {
        return m_mask;
    }

    uint16_t neighbour_cost(const Location cell, const Heading heading) const
    {
        if (not is_exit(cell, heading))
        {
            return MAX_COST; // returns the MAX_COST if the heading in the cell has a wall
        }
        Location next_cell = cell.neighbour(heading);
        return m_cost[next_cell.x][next_cell.y];
    }

    uint16_t cost(const Location cell) const
    {
        return m_cost[cell.x][cell.y];
    }

    void flood(const Location target)
    {
        for (int x = 0; x < MAZE_WIDTH; x++)
        {
            for (int y = 0; y < MAZE_HEIGHT; y++)
            {
                m_cost[x][y] = (uint8_t)MAX_COST;
            }
        }

        // flood is starting from the target cell. after a searching round and reaching the
        // target cell, flood operation will be conducted
        // first cell in the queue will be the target cell
        Queue<Location, QUEUE_SIZE> queue;
        m_cost[target.x][target.y] = 0;
        queue.add(target);
        API::setText(target.x, target.y, std::to_string(0));
        while (queue.size() > 0)
        {
            // sets the current flood location from the here, initially it's the target
            Location here = queue.head();
            // before searching for a EXIT and move there, cost is incremented
            uint16_t newCost = m_cost[here.x][here.y] + 1;

            // the casting of enums is potentially problematic
            // check all 4 walls while staying in the current flood location
            for (int h = NORTH; h < HEADING_COUNT; h++)
            {
                Heading heading = static_cast<Heading>(h);
                // checks whether the current flood cell has a EXIT in any wall
                if (is_exit(here, heading))
                {
                    // the neigbour will return the coordinate for the opened wall will lead
                    Location nextCell = here.neighbour(heading);
                    // checks whether the cell already have a cost value assigned to it.
                    // if the assigned value is smaller than the current assigning newCost, then the cost for that cell will be
                    // the newCost
                    if (m_cost[nextCell.x][nextCell.y] > newCost)
                    {
                        m_cost[nextCell.x][nextCell.y] = newCost;
                        API::setText(nextCell.x, nextCell.y, std::to_string(newCost));
                        // the neigbour cell is added to the queue so that next iteration of the flood can use
                        // this cell as the current flood cell
                        queue.add(nextCell);
                    }
                }
            }
        }
    }

    Heading heading_to_smallest(const Location cell, const Heading start_heading) const
    {
        Heading next_heading = start_heading;
        Heading best_heading = BLOCKED;
        uint16_t best_cost = cost(cell);
        uint16_t cost;
        cost = neighbour_cost(cell, next_heading);
        if (cost < best_cost)
        {
            best_cost = cost;
            best_heading = next_heading;
        };
        next_heading = right_from(start_heading);
        cost = neighbour_cost(cell, next_heading);
        if (cost < best_cost)
        {
            best_cost = cost;
            best_heading = next_heading;
        };
        next_heading = left_from(start_heading);
        cost = neighbour_cost(cell, next_heading);
        if (cost < best_cost)
        {
            best_cost = cost;
            best_heading = next_heading;
        };
        next_heading = behind_from(start_heading);
        cost = neighbour_cost(cell, next_heading);
        if (cost < best_cost)
        {
            best_cost = cost;
            best_heading = next_heading;
        };
        if (best_cost == MAX_COST)
        {
            best_heading = BLOCKED;
        }
        return best_heading;
    }

private:
    // Unconditionally set a wall state.
    // use update_wall_state() when exploring
    void set_wall_state(const Location loc, const Heading heading, const WallState state)
    {
        switch (heading)
        {
        case NORTH:
            m_walls[loc.x][loc.y].north = state;
            m_walls[loc.north().x][loc.north().y].south = state;
            break;
        case EAST:
            m_walls[loc.x][loc.y].east = state;
            m_walls[loc.east().x][loc.east().y].west = state;
            break;
        case WEST:
            m_walls[loc.x][loc.y].west = state;
            m_walls[loc.west().x][loc.west().y].east = state;
            break;
        case SOUTH:
            m_walls[loc.x][loc.y].south = state;
            m_walls[loc.south().x][loc.south().y].north = state;
            break;
        default:
            // ignore any other heading (blocked)
            break;
        }
    }
    MazeMask m_mask = MASK_OPEN;
    Location m_goal = TARGET;

    uint8_t m_cost[MAZE_WIDTH][MAZE_HEIGHT];
    WallInfo m_walls[MAZE_WIDTH][MAZE_HEIGHT];
};

extern Maze maze;

#endif
