#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>
#include <thread>
#include <vector>

static const uint32_t NUM_ROWS = 15;

// Constants
const uint32_t PLANT_MAXIMUM_AGE = 10;
const uint32_t HERBIVORE_MAXIMUM_AGE = 50;
const uint32_t CARNIVORE_MAXIMUM_AGE = 80;
const uint32_t MAXIMUM_ENERGY = 200;
const uint32_t THRESHOLD_ENERGY_FOR_REPRODUCTION = 20;

// Probabilities
const double PLANT_REPRODUCTION_PROBABILITY = 0.2;
const double HERBIVORE_REPRODUCTION_PROBABILITY = 0.075;
const double CARNIVORE_REPRODUCTION_PROBABILITY = 0.025;
const double HERBIVORE_MOVE_PROBABILITY = 0.7;
const double HERBIVORE_EAT_PROBABILITY = 0.9;
const double CARNIVORE_MOVE_PROBABILITY = 0.5;
const double CARNIVORE_EAT_PROBABILITY = 1.0;

// Type definitions
enum entity_type_t
{
    empty,
    plant,
    herbivore,
    carnivore
};

struct pos_t
{
    uint32_t i;
    uint32_t j;
};

struct entity_t
{
    entity_type_t type;
    int32_t energy;
    int32_t age;
};

// Auxiliary code to convert the entity_type_t enum to a string
NLOHMANN_JSON_SERIALIZE_ENUM(entity_type_t, {
                                                {empty, " "},
                                                {plant, "P"},
                                                {herbivore, "H"},
                                                {carnivore, "C"},
                                            })

// Auxiliary code to convert the entity_t struct to a JSON object
namespace nlohmann
{
    void to_json(nlohmann::json &j, const entity_t &e)
    {
        j = nlohmann::json{{"type", e.type}, {"energy", e.energy}, {"age", e.age}};
    }
}

// Grid that contains the entities
static std::vector<std::vector<entity_t>> entity_grid;

bool random_action(double probability)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen) < probability;
}

uint32_t i, j;
std::vector<pos_t> valid_pos;
pos_t pos_aleatoria;

pos_t check_empty(pos_t pos)
{
    if (pos.i + 1 < NUM_ROWS)
    {
        if (entity_grid[pos.i + 1][pos.j].type == empty)
        {
            valid_pos.push_back({pos.i + 1, pos.j});
        }
    }
    if (pos.i > 0)
    {
        if (entity_grid[pos.i - 1][pos.j].type == empty)
        {
            valid_pos.push_back({pos.i - 1, pos.j});
        }
    }
    if (pos.j + 1 < NUM_ROWS)
    {
        if (entity_grid[pos.i][pos.j + 1].type == empty)
        {
            valid_pos.push_back({pos.i, pos.j + 1});
        }
    }
    if (pos.j > 0)
    {
        if (entity_grid[pos.i][pos.j - 1].type == empty)
        {
            valid_pos.push_back({pos.i, pos.j - 1});
        }
    }
    if (!valid_pos.empty())
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, valid_pos.size() - 1);
        pos_aleatoria = valid_pos[dis(gen)];
        valid_pos.clear();
        return pos_aleatoria;
    }
    else
    {
        return pos;
    }
}

pos_t check_plants(pos_t pos)
{
    if (pos.i + 1 < NUM_ROWS)
    {
        if (entity_grid[pos.i + 1][pos.j].type == plant)
        {
            valid_pos.push_back({pos.i + 1, pos.j});
        }
    }
    if (pos.i > 0)
    {
        if (entity_grid[pos.i - 1][pos.j].type == plant)
        {
            valid_pos.push_back({pos.i - 1, pos.j});
        }
    }
    if (pos.j + 1 < NUM_ROWS)
    {
        if (entity_grid[pos.i][pos.j + 1].type == plant)
        {
            valid_pos.push_back({pos.i, pos.j + 1});
        }
    }
    if (pos.j > 0)
    {
        if (entity_grid[pos.i][pos.j - 1].type == plant)
        {
            valid_pos.push_back({pos.i, pos.j - 1});
        }
    }
    if (!valid_pos.empty())
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, valid_pos.size() - 1);
        pos_aleatoria = valid_pos[dis(gen)];
        valid_pos.clear();
        return pos_aleatoria;
    }
    else
    {
        return pos;
    }
}

pos_t check_herbivores(pos_t pos)
{
    if (pos.i + 1 < NUM_ROWS)
    {
        if (entity_grid[pos.i + 1][pos.j].type == herbivore)
        {
            valid_pos.push_back({pos.i + 1, pos.j});
        }
    }
    if (pos.i > 0)
    {
        if (entity_grid[pos.i - 1][pos.j].type == herbivore)
        {
            valid_pos.push_back({pos.i - 1, pos.j});
        }
    }
    if (pos.j + 1 < NUM_ROWS)
    {
        if (entity_grid[pos.i][pos.j + 1].type == herbivore)
        {
            valid_pos.push_back({pos.i, pos.j + 1});
        }
    }
    if (pos.j > 0)
    {
        if (entity_grid[pos.i][pos.j - 1].type == herbivore)
        {
            valid_pos.push_back({pos.i, pos.j - 1});
        }
    }
    if (!valid_pos.empty())
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, valid_pos.size() - 1);
        pos_aleatoria = valid_pos[dis(gen)];
        valid_pos.clear();
        return pos_aleatoria;
    }
    else
    {
        return pos;
    }
}



void actions()
{
    for (int i = 0; i < NUM_ROWS; i++)
    {
        for (int j = 0; j < NUM_ROWS; j++)
        {

            if (entity_grid[i][j].type != empty)
            {
                if (entity_grid[i][j].type == plant && entity_grid[i][j].age == PLANT_MAXIMUM_AGE ||
                    entity_grid[i][j].type == herbivore && (entity_grid[i][j].age == HERBIVORE_MAXIMUM_AGE || entity_grid[i][j].energy == 0) ||
                    entity_grid[i][j].type == carnivore && (entity_grid[i][j].age == CARNIVORE_MAXIMUM_AGE || entity_grid[i][j].energy == 0))
                {
                    entity_grid[i][j].type = empty;
                    entity_grid[i][j].age = 0;
                }
                else
                {
                    entity_grid[i][j].age++;
                }

                if (entity_grid[i][j].type == plant)
                {
                    pos_t pos_plant;
                    pos_plant.i = i;
                    pos_plant.j = j;
                    if (random_action(PLANT_REPRODUCTION_PROBABILITY))
                    {
                        pos_t nova_posp = check_empty(pos_plant);
                        if (entity_grid[nova_posp.i][nova_posp.j].type == empty)
                        {
                            entity_grid[nova_posp.i][nova_posp.j].type = plant;
                            entity_grid[nova_posp.i][nova_posp.j].age = 0;
                        }
                    }
                }

                if (entity_grid[i][j].type == herbivore)
                {
                    pos_t pos;
                    pos.i = i;
                    pos.j = j;
                    pos_t eat_plant = check_plants(pos);
                    if(eat_plant.i != pos.i || eat_plant.j != pos.j){
                        if (random_action(HERBIVORE_EAT_PROBABILITY)){
                            entity_grid[eat_plant.i][eat_plant.j].type = herbivore; 
                            entity_grid[eat_plant.i][eat_plant.j].energy = entity_grid[i][j].energy + 30;
                            entity_grid[eat_plant.i][eat_plant.j].age = entity_grid[i][j].age;
                            entity_grid[i][j].type = empty;
                        }
                    }
                    else if (random_action(HERBIVORE_MOVE_PROBABILITY)){
                        pos_t nova_pos = check_empty(pos);
                        if (entity_grid[nova_pos.i][nova_pos.j].type == empty )
                        {
                            entity_grid[nova_pos.i][nova_pos.j].type = herbivore;
                            entity_grid[nova_pos.i][nova_pos.j].age = entity_grid[i][j].age;
                            entity_grid[nova_pos.i][nova_pos.j].energy = entity_grid[i][j].energy - 5;
                            entity_grid[i][j].type = empty;
                        }
                    }
                    else if (entity_grid[i][j].energy > THRESHOLD_ENERGY_FOR_REPRODUCTION && random_action(HERBIVORE_REPRODUCTION_PROBABILITY))
                    {
                        if((random_action(HERBIVORE_REPRODUCTION_PROBABILITY))){
                        entity_grid[i][j].energy = entity_grid[i][j].energy - 10;
                        pos_t nova_pos = check_empty(pos);
                            if (entity_grid[nova_pos.i][nova_pos.j].type == empty)
                            {
                                entity_grid[nova_pos.i][nova_pos.j].type = herbivore;
                                entity_grid[nova_pos.i][nova_pos.j].age = 0;
                                entity_grid[nova_pos.i][nova_pos.j].energy = 50;
                            }
                        }
                    }
                }
                
                if (entity_grid[i][j].type == carnivore)
                {
                    pos_t pos;
                    pos.i = i;
                    pos.j = j;
                    if (entity_grid[i][j].energy > THRESHOLD_ENERGY_FOR_REPRODUCTION && random_action(CARNIVORE_REPRODUCTION_PROBABILITY))
                    {
                        entity_grid[i][j].energy = entity_grid[i][j].energy - 10;
                        pos_t nova_pos = check_empty(pos);
                        if (entity_grid[nova_pos.i][nova_pos.j].type == empty)
                        {
                            entity_grid[nova_pos.i][nova_pos.j].type = carnivore;
                            entity_grid[nova_pos.i][nova_pos.j].age = 0;
                            entity_grid[nova_pos.i][nova_pos.j].energy = 100;
                        }
                    }
                    else if (random_action(CARNIVORE_MOVE_PROBABILITY)){
                        pos_t nova_pos = check_empty(pos);
                        if (entity_grid[nova_pos.i][nova_pos.j].type == empty )
                        {
                            entity_grid[nova_pos.i][nova_pos.j].type = carnivore;
                            entity_grid[nova_pos.i][nova_pos.j].age = entity_grid[i][j].age;
                            entity_grid[nova_pos.i][nova_pos.j].energy = entity_grid[i][j].energy - 5;
                            entity_grid[i][j].type = empty;
                        }
                    }
                    pos_t eat_herb = check_herbivores(pos);
                    if(eat_herb.i != pos.i || eat_herb.j != pos.j){
                        if (random_action(CARNIVORE_EAT_PROBABILITY)){
                            entity_grid[eat_herb.i][eat_herb.j].type = carnivore; 
                            entity_grid[eat_herb.i][eat_herb.j].energy = entity_grid[i][j].energy + 30;
                            entity_grid[eat_herb.i][eat_herb.j].age = entity_grid[i][j].age;
                            entity_grid[i][j].type = empty;
                        }
                    }
                }
            }
        }
    }
}

int main()
{
    crow::SimpleApp app;

    // Endpoint to serve the HTML page
    CROW_ROUTE(app, "/")
    ([](crow::request &, crow::response &res)
     {
        // Return the HTML content here
        res.set_static_file_info_unsafe("../public/index.html");
        res.end(); });

    CROW_ROUTE(app, "/start-simulation")
        .methods("POST"_method)([](crow::request &req, crow::response &res)
                                { 
        // Parse the JSON request body
        nlohmann::json request_body = nlohmann::json::parse(req.body);

       // Validate the request body 
        uint32_t total_entinties = (uint32_t)request_body["plants"] + (uint32_t)request_body["herbivores"] + (uint32_t)request_body["carnivores"];
        if (total_entinties > NUM_ROWS * NUM_ROWS) {
        res.code = 400;
        res.body = "Too many entities";
        res.end();
        return;
        }

        // Clear the entity grid
        entity_grid.clear();
        entity_grid.assign(NUM_ROWS, std::vector<entity_t>(NUM_ROWS, { empty, 0, 0}));
        
        // Create the entities
        // <YOUR CODE HERE>
        uint32_t num_plants= (uint32_t)request_body["plants"];
        uint32_t num_herbivores= (uint32_t)request_body["herbivores"];
        uint32_t num_carnivores= (uint32_t)request_body["carnivores"];

        int count_p=0;
        int count_h=0;
        int count_c=0; 

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 14);

        while(count_p< num_plants){
            i= dis(gen);
            j= dis(gen);

            while(entity_grid[i][j].type!= empty){
                i= dis(gen);
                j= dis(gen);
            }
            entity_grid[i][j].type= plant;
            entity_grid[i][j].age= 0;
            count_p++;
        }

        while(count_h< num_herbivores){
            pos_t pos_herbivore;
            i= dis(gen);
            j= dis(gen);

            while(entity_grid[i][j].type!= empty){
                i= dis(gen);
                j= dis(gen);
            }
            entity_grid[i][j].type= herbivore;
            entity_grid[i][j].age= 0;
            entity_grid[i][j].energy= 50;
            count_h++;
        }

        while(count_c< num_carnivores){
            pos_t pos_carnivore; 
            i= dis(gen);
            j= dis(gen);

            while(entity_grid[i][j].type!= empty){
                i= dis(gen);
                j= dis(gen);
            }
            entity_grid[i][j].type= carnivore;
            entity_grid[i][j].age= 0;
            entity_grid[i][j].energy= 100;
            count_c++;
        }

        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        res.body = json_grid.dump();
        res.end(); });

    // Endpoint to process HTTP GET requests for the next simulation iteration
    CROW_ROUTE(app, "/next-iteration")
        .methods("GET"_method)([]()
                               {
        // Simulate the next iteration
        // Iterate over the entity grid and simulate the behaviour of each entity
        
        // <YOUR CODE HERE> 
        actions();
        
        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        return json_grid.dump(); });
    app.port(8080).run();

    return 0;
}
