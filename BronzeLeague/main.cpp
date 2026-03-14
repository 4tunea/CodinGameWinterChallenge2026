#include <bits/stdc++.h>

/** TO DO LIST
*  
*  
*  
**/

using namespace std;

struct Coord{
    int x {};                //  x coordinate
    int y {};                //  y coordinate

    bool operator==(const Coord& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Coord& other) const {
        return x != other.x || y != other.y;
    }
};

struct Game{
    int myId {};              //  my id
    int width {};             //  map width
    int height {};            //  map height
    int snPP {};              //  snakebots per player
    int pwCnt {};             //  power source count
    int snCnt {};             //  snakebot count
};

struct Snake{
    int id {};                //  snake id
    string body {};           //  full snake body line
    Coord h {};               //  snake head
    vector<Coord> b {};       //  snake body
    string nextMove {"WAIT"}; //  next move of snake
    bool mySnake {};          //  does snake belong to me
};

//  --------------  DEBUG

void PRINT(const vector<Snake> & s){
    for(auto i : s){
        cerr << "id: " << i.id
        << "\nbody: " << i.body
        << "\nh: " << i.h.x << " " << i.h.y
        << "\nb: ";
        for(auto j : i.b){
            cerr << j.x << ":" << j.y << " | ";
        }
        cerr << "\nnextM: " << i.nextMove
        << "\nisMySnake: " << i.mySnake;
        cerr << "\n\n";
    }
}

void PRINT(const Game & g){
    cerr << "MyID: " << g.myId << '\n';
    cerr << "Width: " << g.width << '\n';
    cerr << "Height: " << g.height << '\n';
    cerr << "SnakesPerPlayer: " << g.snPP << '\n';
    cerr << "PowerCount: " << g.pwCnt << '\n';
    cerr << "SnakeCount: " << g.snCnt << '\n';
    cerr << "\n";
}

void PRINT(const vector<string> & map){
    for(auto i : map){
        cerr << i << '\n';
    }
    cerr<<'\n';
}
//  --------------  DEBUG END

vector<Coord> bodyMove(const vector<string> & map, const vector<Coord> & b, Coord d){
    vector<Coord> body {b};
    for(size_t i {}; i < b.size(); ++i){
        if(i == 0){
            body[i] = {b[i].x + d.x, b[i].y + d.y};
            continue;
        }
        body[i] = {b[i-1].x, b[i-1].y};
    }
    return body;
}

string simpleBfs(const Game & game, const vector<string> & map, const Snake & startSnake, int maxDepth){
    string move {"WAIT"};
    
    struct Que{
        Snake snake {};
        double points {};            // power = 1, move = -0.01
        Coord origin {};
        int depth {};
    };
    queue<Que> que;
    que.push({startSnake, 100, {-1, -1}, 0});
    double maxPointsFound {};
    while(!que.empty()){
        Que q {que.front()};
        que.pop();
        if(q.points > maxPointsFound && startSnake.h != q.snake.h){
            maxPointsFound = q.points;
            if(q.origin.x == startSnake.h.x){
                if(q.origin.y > startSnake.h.y){
                    move = "DOWN";
                } else {
                    move = "UP";
                }
            }else{
                if(q.origin.x > startSnake.h.x){
                    move = "RIGHT";
                } else {
                    move = "LEFT";
                }
            }
        }
        if(q.depth >= maxDepth){
            continue;
        }

        Coord dir[4] {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for(auto d : dir){
            bool valid {1};
            if(q.snake.h.x + d.x == game.width || q.snake.h.x + d.x == -1 || q.snake.h.y + d.y == game.height || q.snake.h.y + d.y == -1) valid = 0;
            if(map[q.snake.h.y + d.y][q.snake.h.x + d.x] == '#' || map[q.snake.h.y + d.y][q.snake.h.x + d.x] == '0') valid = 0;
            for(size_t i {}; i < q.snake.b.size() - 1; ++i){   //  -1 because skip last body part (it moves)
                if(q.snake.b[i].x == q.snake.h.x + d.x && q.snake.b[i].y == q.snake.h.y + d.y){
                    valid = 0;
                }
            }
            if(q.snake.h.x + d.x == q.snake.b[1].x && q.snake.h.y + d.y == q.snake.b[1].y) valid = 0;
            if(!valid) continue;
            Snake tempSn {q.snake};
            double tempPw {q.points - 0.01};  // move penalty
            if(map[q.snake.h.y + d.y][q.snake.h.x + d.x] == 'P') tempPw += 1;
            tempSn.h.y += d.y;
            tempSn.h.x += d.x;
            tempSn.b = bodyMove(map, q.snake.b, d);
            if(q.origin.x == -1){
                que.push({tempSn, tempPw, tempSn.h, q.depth + 1});
            } else {
                que.push({tempSn, tempPw, q.origin, q.depth + 1});
            }
        }
    }

    return move;
}

int main()
{
    Game game {};
    vector<string> field {};
    vector<int> mySn {};
    vector<int> oppSn {};

    cin >> game.myId >> game.width >> game.height; cin.ignore();
    for (size_t i {}; i < game.height; ++i) {
        string row {};
        getline(cin, row);
        field.push_back(row);
    }
    cin >> game.snPP; cin.ignore();
    for (size_t i {}; i < game.snPP; ++i) {
        int my_snakebot_id {};
        cin >> my_snakebot_id; cin.ignore();
        mySn.push_back(my_snakebot_id);
    }
    for (size_t i {}; i < game.snPP; ++i) {
        int opp_snakebot_id {};
        cin >> opp_snakebot_id; cin.ignore();
        oppSn.push_back(opp_snakebot_id);
    }

    // game loop
    while (1) {
        vector<string> map {field};
        vector<Coord> powers {};
        vector<Snake> snake {};

        cin >> game.pwCnt; cin.ignore();
        for (size_t i {}; i < game.pwCnt; ++i) {
            Coord c {};
            cin >> c.x >> c.y; cin.ignore();
            powers.push_back(c);
            map[c.y][c.x] = 'P';
        }

        cin >> game.snCnt; cin.ignore();
        for (size_t i {}; i < game.snCnt; ++i) {
            int snakebot_id {};
            string body {};
            cin >> snakebot_id >> body; cin.ignore();
            vector<Coord> b {};
            stringstream ss {body};
            string eil {};
            while(getline(ss, eil, ':')){
                stringstream sss {eil};
                string x {}, y {};
                getline(sss, x, ',');
                getline(sss, y, ',');
                b.push_back({stoi(x), stoi(y)});
            }
            bool mySnake {};
            for(auto i : mySn){
                if(i == snakebot_id){
                    mySnake = true;
                    break;
                }
            }
            snake.push_back({snakebot_id, body, b[0], b, "WAIT", mySnake});
        }

        //  ----------  MAIN LOGIC
        for(auto & i : snake){
            if(!i.mySnake) continue;
            vector<string> mapSn {map};
            for(auto j : snake){
                if(i.id == j.id) continue;
                for(size_t z {}; z < j.b.size() - 1; ++z){       // -1 because we dont need the last body part (it moves)
                    mapSn[j.b[z].y][j.b[z].x] = '0';
                }
            }
            PRINT(mapSn);
            i.nextMove = simpleBfs(game, mapSn, i, 3);
        }
        
        string command {};
        for(auto i : snake){
            if(!i.mySnake) continue;
            if(i.nextMove == "WAIT"){
                command = command + i.nextMove + ';';
            }else{
                command = command + to_string(i.id) + ' ' + i.nextMove + ';';
            }
        }
        cout << command << endl;

        //PRINT(map);
        //PRINT(game);
        //PRINT(snake);
    }
}
