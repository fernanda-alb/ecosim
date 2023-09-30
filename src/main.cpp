#define CROW_MAIN
#define CROW_STATIC_DIR "../public"

#include "crow_all.h"
#include "json.hpp"
#include <random>
#include <thread>
#include <vector>
#include <mutex>

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
    std::mutex* m;
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
std::vector<pos_t> valid_pos, occupied_pos;
pos_t pos_aleatoria, pos, current_pos;

std::vector<std::thread> threads;

//pos_t eat_plant = check_plants(pos, occupied_pos);
// pos_t eat_herb = check_herbivores(pos, occupied_pos);

bool check_availability(pos_t current_pos, std::vector<pos_t> occupied_pos)
{
    for (auto &it : occupied_pos)
    {
        if (it.i == current_pos.i && it.j == current_pos.j)
        {
            return false;
        }
    }
    return true;
}

void lock(pos_t pos){
    entity_grid[pos.i][pos.j].m-> lock();  
    if (pos.i + 1 < NUM_ROWS)
    {
        entity_grid[pos.i+1][pos.j].m-> lock();   
    }
    if (pos.i > 0)
    {
        entity_grid[pos.i-1][pos.j].m-> lock();
    }
    if (pos.j + 1 < NUM_ROWS)
    {
        entity_grid[pos.i][pos.j+1].m-> lock();
    }
    if (pos.j > 0)
    {
        entity_grid[pos.i][pos.j-1].m-> lock();
    }
}

void unlock(pos_t pos){
    entity_grid[pos.i][pos.j].m-> unlock();  
    if (pos.i + 1 < NUM_ROWS)
    {
        entity_grid[pos.i+1][pos.j].m-> unlock();   
    }
    if (pos.i > 0)
    {
        entity_grid[pos.i-1][pos.j].m-> unlock();
    }
    if (pos.j + 1 < NUM_ROWS)
    {
        entity_grid[pos.i][pos.j+1].m-> unlock();
    }
    if (pos.j > 0)
    {
        entity_grid[pos.i][pos.j-1].m-> unlock();
    }  
}

pos_t check_empty(pos_t pos, std::vector<pos_t> occupied_pos)
{
    pos_t aux;
    if (pos.i + 1 < NUM_ROWS)
    {
        aux.i = i + 1;
        aux.j = j;

        if (entity_grid[pos.i + 1][pos.j].type == empty && check_availability(aux, occupied_pos))
        {
            valid_pos.push_back({pos.i + 1, pos.j});
        }
    }
    if (pos.i > 0)
    {
        aux.i = i - 1;
        aux.j = j;

        if (entity_grid[pos.i - 1][pos.j].type == empty && check_availability(aux, occupied_pos))
        {
            valid_pos.push_back({pos.i - 1, pos.j});
        }
    }
    if (pos.j + 1 < NUM_ROWS)
    {
        aux.i = i;
        aux.j = j + 1;

        if (entity_grid[pos.i][pos.j + 1].type == empty && check_availability(aux, occupied_pos))
        {
            valid_pos.push_back({pos.i, pos.j + 1});
        }
    }
    if (pos.j > 0)
    {
        aux.i = i;
        aux.j = j - 1;

        if (entity_grid[pos.i][pos.j - 1].type == empty && check_availability(aux, occupied_pos))
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

// pos_t check_plants(pos_t pos, std::vector<pos_t> occupied_pos)
// {
//     pos_t aux;
//     if (pos.i + 1 < NUM_ROWS)
//     {
//         entity_grid[pos.i + 1][pos.j].m.lock();
//         aux.i = i + 1;
//         aux.j = j;

//         if (entity_grid[pos.i + 1][pos.j].type == plant && check_availability(aux, occupied_pos))
//         {
//             valid_pos.push_back({pos.i + 1, pos.j});
//         }
//     }
//     if (pos.i > 0)
//     {
//         entity_grid[pos.i - 1][pos.j].m.lock();
//         aux.i = i - 1;
//         aux.j = j;

//         if (entity_grid[pos.i - 1][pos.j].type == plant && check_availability(aux, occupied_pos))
//         {
//             valid_pos.push_back({pos.i - 1, pos.j});
//         }
//     }
//     if (pos.j + 1 < NUM_ROWS)
//     {
//         entity_grid[pos.i][pos.j+1].m.lock();
//         aux.i = i;
//         aux.j = j + 1;

//         if (entity_grid[pos.i][pos.j + 1].type == plant && check_availability(aux, occupied_pos))
//         {
//             valid_pos.push_back({pos.i, pos.j + 1});
//         }
//     }
//     if (pos.j > 0)
//     {
//         entity_grid[pos.i][pos.j-1].m.lock();
//         aux.i = i;
//         aux.j = j - 1;

//         if (entity_grid[pos.i][pos.j - 1].type == plant && check_availability(aux, occupied_pos))
//         {
//             valid_pos.push_back({pos.i, pos.j - 1});
//         }
//     }
//     if (!valid_pos.empty())
//     {
//         static std::random_device rd;
//         static std::mt19937 gen(rd());
//         std::uniform_int_distribution<> dis(0, valid_pos.size() - 1);
//         pos_aleatoria = valid_pos[dis(gen)];
//         valid_pos.clear();
//         return pos_aleatoria;
//     }
//     else
//     {
//         return pos;
//     }
// }

// pos_t check_herbivores(pos_t pos, std::vector<pos_t> occupied_pos)
// {
//     pos_t aux;
//     if (pos.i + 1 < NUM_ROWS)
//     {
//         entity_grid[pos.i+1][pos.j].m.lock();
//         aux.i = i + 1;
//         aux.j = j;
//         if (entity_grid[pos.i + 1][pos.j].type == herbivore && check_availability(aux, occupied_pos))
//         {
//             valid_pos.push_back({pos.i + 1, pos.j});
//         }
//     }
//     if (pos.i > 0)
//     {
//         entity_grid[pos.i-1][pos.j].m.lock();
//         aux.i = i - 1;
//         aux.j = j;
//         if (entity_grid[pos.i - 1][pos.j].type == herbivore && check_availability(aux, occupied_pos))
//         {
//             valid_pos.push_back({pos.i - 1, pos.j});
//         }
//     }
//     if (pos.j + 1 < NUM_ROWS)
//     {
//         entity_grid[pos.i][pos.j+1].m.lock();
//         aux.i = i;
//         aux.j = j + 1;
//         if (entity_grid[pos.i][pos.j + 1].type == herbivore && check_availability(aux, occupied_pos))
//         {
//             valid_pos.push_back({pos.i, pos.j + 1});
//         }
//     }
//     if (pos.j > 0)
//     {
//         entity_grid[pos.i][pos.j-1].m.lock();
//         aux.i = i;
//         aux.j = j - 1;
//         if (entity_grid[pos.i][pos.j - 1].type == herbivore && check_availability(aux, occupied_pos))
//         {
//             valid_pos.push_back({pos.i, pos.j - 1});
//         }
//     }
//     if (!valid_pos.empty())
//     {
//         static std::random_device rd;
//         static std::mt19937 gen(rd());
//         std::uniform_int_distribution<> dis(0, valid_pos.size() - 1);
//         pos_aleatoria = valid_pos[dis(gen)];
//         valid_pos.clear();
//         return pos_aleatoria;
//     }
//     else
//     {
//         return pos;
//     }
// }

// pos_t eat_plant = check_plants(pos, occupied_pos);
// pos_t eat_herb = check_herbivores(pos, occupied_pos);

void simulate_plant(pos_t pos)
{
    lock(pos);
    if (random_action(PLANT_REPRODUCTION_PROBABILITY))
    {
        pos_t nova_pos = check_empty(pos, occupied_pos);
        if (entity_grid[nova_pos.i][nova_pos.j].type == empty)
        {
            entity_grid[nova_pos.i][nova_pos.j].type = plant;
            entity_grid[nova_pos.i][nova_pos.j].age = 0;
            occupied_pos.push_back(nova_pos);
        }
    }
    unlock(pos);  
}

// void simulate_herbivore(pos_t pos)
// {
//     if (entity_grid[i][j].energy > THRESHOLD_ENERGY_FOR_REPRODUCTION && random_action(HERBIVORE_REPRODUCTION_PROBABILITY))
//     {
//         if ((random_action(HERBIVORE_REPRODUCTION_PROBABILITY)))
//         {
//             entity_grid[i][j].energy = entity_grid[i][j].energy - 10;
//             pos_t nova_pos = check_empty(pos, occupied_pos);
//             if (entity_grid[nova_pos.i][nova_pos.j].type == empty)
//             {
//                 entity_grid[nova_pos.i][nova_pos.j].type = herbivore;
//                 entity_grid[nova_pos.i][nova_pos.j].age = 0;
//                 entity_grid[nova_pos.i][nova_pos.j].energy = 100;
//                 occupied_pos.push_back(nova_pos);
//             }
//         }
//     }
//     else if (eat_plant.i != pos.i || eat_plant.j != pos.j)
//     {
//         if (random_action(HERBIVORE_EAT_PROBABILITY))
//         {
//             entity_grid[eat_plant.i][eat_plant.j].type = herbivore;
//             entity_grid[eat_plant.i][eat_plant.j].age = entity_grid[i][j].age;

//             if (entity_grid[i][j].energy <= MAXIMUM_ENERGY - 30)
//             {
//                 entity_grid[eat_plant.i][eat_plant.j].energy = entity_grid[i][j].energy + 30;
//             }
//             else
//             {
//                 entity_grid[eat_plant.i][eat_plant.j].energy = MAXIMUM_ENERGY;
//             }
//             entity_grid[i][j].type = empty;
//             occupied_pos.push_back(eat_plant);
//         }
//     }
//     else if (random_action(HERBIVORE_MOVE_PROBABILITY))
//     {
//         pos_t nova_pos = check_empty(pos, occupied_pos);
//         if (entity_grid[nova_pos.i][nova_pos.j].type == empty)
//         {
//             entity_grid[nova_pos.i][nova_pos.j].type = herbivore;
//             entity_grid[nova_pos.i][nova_pos.j].age = entity_grid[i][j].age;
//             entity_grid[nova_pos.i][nova_pos.j].energy = entity_grid[i][j].energy - 5;
//             entity_grid[i][j].type = empty;
//             occupied_pos.push_back(nova_pos);
//         }
//     }
// }

// void simulate_carnivore(pos_t pos)
// {
//     if (entity_grid[i][j].energy > THRESHOLD_ENERGY_FOR_REPRODUCTION && random_action(CARNIVORE_REPRODUCTION_PROBABILITY))
//     {
//         entity_grid[i][j].energy = entity_grid[i][j].energy - 10;
//         pos_t nova_pos = check_empty(pos, occupied_pos);
//         if (entity_grid[nova_pos.i][nova_pos.j].type == empty)
//         {
//             entity_grid[nova_pos.i][nova_pos.j].type = carnivore;
//             entity_grid[nova_pos.i][nova_pos.j].age = 0;
//             entity_grid[nova_pos.i][nova_pos.j].energy = 100;
//             occupied_pos.push_back(nova_pos);
//         }
//     }
//     pos_t eat_herb = check_herbivores(pos, occupied_pos);
//     if (eat_herb.i != pos.i || eat_herb.j != pos.j)
//     {
//         if (random_action(CARNIVORE_EAT_PROBABILITY))
//         {
//             entity_grid[eat_herb.i][eat_herb.j].type = carnivore;
//             if (entity_grid[i][j].energy <= MAXIMUM_ENERGY - 20)
//             {
//                 entity_grid[eat_herb.i][eat_herb.j].energy = entity_grid[i][j].energy + 20;
//             }
//             else
//             {
//                 entity_grid[eat_herb.i][eat_herb.j].energy = MAXIMUM_ENERGY;
//             }
//             entity_grid[eat_herb.i][eat_herb.j].age = entity_grid[i][j].age;
//             entity_grid[i][j].type = empty;
//             occupied_pos.push_back(eat_herb);
//         }
//     }
//     else if (random_action(CARNIVORE_MOVE_PROBABILITY))
//     {
//         pos_t nova_pos = check_empty(pos, occupied_pos);
//         if (entity_grid[nova_pos.i][nova_pos.j].type == empty)
//         {
//             entity_grid[nova_pos.i][nova_pos.j].type = carnivore;
//             entity_grid[nova_pos.i][nova_pos.j].age = entity_grid[i][j].age;
//             entity_grid[nova_pos.i][nova_pos.j].energy = entity_grid[i][j].energy - 5;
//             entity_grid[i][j].type = empty;
//             occupied_pos.push_back(nova_pos);
//         }
//     }
// }

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
        entity_grid.assign(NUM_ROWS, std::vector<entity_t>(NUM_ROWS, { empty, 0, 0, new std::mutex()}));
        for(i=0; i< NUM_ROWS; i++){
            for(j=0; j<NUM_ROWS; j++){
                entity_grid[i][j].m = (new std::mutex);
            }
        }
    

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
            entity_grid[i][j].energy= 100;
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
    // std::vector<pos_t> occupied_pos; 
    // pos_t current_pos;

    for (int i = 0; i < NUM_ROWS; i++)
    {
        for (int j = 0; j < NUM_ROWS; j++)
        {
            current_pos.i = i;
            current_pos.j = j;
            if(check_availability(current_pos, occupied_pos)){
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
                        pos_t pos;
                        pos.i = i;
                        pos.j = j;
                        
                        //entity_grid[pos.i][pos.j].m-> lock();   
                        lock(pos);
                        threads.emplace_back(simulate_plant, pos); 
                        // std::thread p(simulate_plant, pos);    
                        // p.join();
                    }
                      

                    // if (entity_grid[i][j].type == herbivore)
                    // {
                    //     pos_t pos;
                    //     pos.i = i;
                    //     pos.j = j;
                    //     entity_grid[i][j].m-> lock();

                    //     std::thread h(simulate_plant, pos);
                    //     h.join();
                    // }
                    
                    // if (entity_grid[i][j].type == carnivore)
                    // {
                    //     pos_t pos;
                    //     pos.i = i;
                    //     pos.j = j;
                    //     entity_grid[i][j].m-> lock();

                    //     std::thread c(simulate_plant, pos);
                    //     c.join();
                    // }
                }
            } 
        }
    }

    for (std::thread& thread : threads)
    {
        thread.join();
    }

        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        return json_grid.dump(); });
    app.port(8080).run();

    return 0;
}
