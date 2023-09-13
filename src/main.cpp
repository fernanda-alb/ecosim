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


bool random_action(float probability) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0); // usar  amesma lógica p/ gerar posicao (real - int e 0-1 vira 0-14)
    return dis(gen) < probability;
}

// Simulate random actions for different entities
void simulate_random_actions() {
    // Probabilities for different actions
    float plant_growth_probability = 0.20; // 20% chance of growth
    float herbivore_move_probability = 0.70; // 70% chance to action
    float carnivore_move_probability = 0.60; // 60% chance to action

    // Simulate plant growth
    if (random_action(plant_growth_probability)) {
        std::cout << "Plant grows.\n";
        // check espacos vazios 
        // pilha 
        // thread nova - cresce aleat
    } else {
        std::cout << "Plant does not grow.\n";
    }

    // Simulate herbivore action
    if (random_action(herbivore_move_probability)) {
        std::cout << "Herbivore moves.\n";
        // check espacos vazios 
        // pilha? - anda aleat
        
    } else {
        std::cout << "Herbivore does not move.\n";
    }

    // Simulate carnivore action
    if (random_action(carnivore_move_probability)) {
        std::cout << "Carnivore moves.\n";
        // check espacos vazios + espacos com herb
        // pilha? - anda aleat
        
    } else {
        std::cout << "Carnivore does not move.\n";
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
        int i, j;

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 14);

        while(count_p< num_plants){
            pos_t pos_plant;
            i= dis(gen);
            j= dis(gen);

            while(entity_grid[i][j].type!= empty){
                i= dis(gen);
                j= dis(gen);
            }
            entity_grid[i][j].type= plant;
            entity_grid[i][j].age= 0;

            pos_plant.i= i;
            pos_plant.j= j; 

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

        //SIMULATE_RANDOM_ACTIONS.CPP
        simulate_random_actions();

        // possibilidades da planta- posições 
            // reprodução- valor aleatório 0-1 : 0.7 chance de crescer
            // crescer- posição adjacente vazia aletória 
            // vida- 10 iterações. Depois morre e esvazia célula

        //possibilidades do herbívoro 
            // walk- valor aleatório 0-1: 0.7 chance 
            // walk- adjacente vazia aleat (exceto com carnívoro)
            // walk- perde 5 de E
            // eat - 0-1: 0.9 chance de comer planta vizinha
            // eat- +30 E
            // reprod.- 0-1: 0.075 chance (quando E>20)
            // reprod.- perde 10 E
            // reprod.- neném surge em cél vazia adjacente aleat. 
            // morte- E=0
            // vida- 50 iterações (se não morrer de fome)

        //possibilidades do carnívoro 
            // walk- 0-1: 0.5 chance 
            // walk- direcao: cél adj aleat (até com herb)
            // walk- perde 5E
            // eat- se adj a um herb, 1 chance de comer 
            // eat- +20E ao comer herb
            // reprod.- 0,025 chance com E>20
            // reprod.- neném surge cél adj vazia aleat
            // morte- E=0
            // vida- 80 iterações se não morrer de fome 
        
        // Return the JSON representation of the entity grid
        nlohmann::json json_grid = entity_grid; 
        return json_grid.dump(); });
    app.port(8080).run();

    return 0;
}

