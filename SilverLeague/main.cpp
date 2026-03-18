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

//  unordered_map hashing voodoo
namespace std {
    template <>
    struct hash<Coord> {
        size_t operator()(const Coord& c) const {
            return hash<int>()(c.x) ^ (hash<int>()(c.y) << 1);
        }
    };
}

struct Game{
    int myId {};              //  my id
    int width {};             //  map width
    int height {};            //  map height
    int snPP {};              //  snakebots per player
    int pwCnt {};             //  power source count
    int snCnt {};             //  snakebot count
    bool firstTurn {true};        //  true if the first turn is active (1s time)
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

void PRINT(const vector<vector<int>> & mp){
    for(auto i : mp){
        for(auto j : i){
            cerr << j << ' ';
        }
        cerr << '\n';
    }
    cerr << '\n';
}
//  --------------  DEBUG END

bool isFalling(const Game & game, const vector<string> & fullMap, const vector<Coord> & b){
    for(auto i : b){
        if(i.y + 1 >= game.height) continue;
        if(fullMap[i.y + 1][i.x] != '.') return false; 
    }
    return true;
}

void doGravityWork(const Game & game, const vector<string> & mapFull, vector<Coord> & b){
    int minFall {INT_MAX};
    map<int, int> checkList;
    for(size_t i {}; i < b.size(); ++i){
        if(checkList[b[i].x] == 0){
            checkList[b[i].x] = b[i].y + 1;
        } else {
            if(checkList[b[i].x] < b[i].y + 1){
                checkList[b[i].x] = b[i].y + 1;
            }
        }
    }

    for(auto i : checkList){
        int y {i.second};
        while(y < game.height){
            if(mapFull[y][i.first] != '.'){
                if(minFall > y - i.second){
                    minFall = y - i.second;
                }
                break;
            }
            y += 1;
        }
    }

    //for(auto i : b){
    //    cerr << "|" <<i.x << "," << i.y<<"|";
    //}
    //cerr<<"\n"<<minFall<<"\n";

    for(auto & i : b){
        i.y += minFall;
    }
}

vector<Coord> bodyMove(const Game & game, const vector<string> & map, const vector<Coord> & b, Coord d, bool tookPower){
    vector<Coord> body {b};
    if(tookPower) body.push_back(body[body.size()-1]);
    body[0] = {b[0].x + d.x, b[0].y + d.y};
    for(size_t i {1}; i < body.size(); ++i){
        if(i-1 < b.size()) {
            body[i] = {b[i-1].x, b[i-1].y};
        } else {
            body[i] = body[i-1];
        }
    }

    if(isFalling(game, map, body)){
        doGravityWork(game, map, body);
    }
    
    //for(auto i : body){cerr << "|" <<i.x << "," << i.y<<"|";}cerr << "\n\n";
    return body;
}

double calculatePoints(const vector<string> & map, bool tookPower, int depth, int  distToPower){
    double points = 0;
    if(tookPower) {
        points += 50000 - depth * 1000;
    }
    points -= distToPower; 
    points += 100;
    return points;
}

string simpleBfs(const Game & game, const vector<string> & mp, const vector<string> & fullMap, const Snake & startSnake, const unordered_map<Coord, vector<vector<int>>> & powerDistanceMap, const vector<Coord> & powers, int maxDepth){
    string move {"WAIT"};
    
    struct Que{
        Snake snake {};
        double points {};            // power = 1, move = -0.01
        Coord origin {};
        int depth {};
    };
    queue<Que> que;
    que.push({startSnake, 10000, {-1, -1}, 0});
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

        Coord dir[4] {{0, 1}, {1, 0}, {-1, 0}, {0, -1}};
        if(q.snake.id % 2 == 0){
            Coord temp = dir[0];
            dir[0] = dir[1];
            dir[1] = dir[2];
            dir[2] = dir[3];
            dir[3] = temp;
        }
        
        int minDistance {INT_MAX};
        Coord target {-1, -1};
        for(auto & i : powers){
            if(minDistance > powerDistanceMap.at(i)[q.snake.h.y][q.snake.h.x]){
                minDistance = powerDistanceMap.at(i)[q.snake.h.y][q.snake.h.x];
                target = {i};
            }
        }

        for(auto & d : dir){
            bool valid {1};
            if(q.snake.h.x + d.x >= game.width || q.snake.h.x + d.x < 0 || q.snake.h.y + d.y >= game.height || q.snake.h.y + d.y < 0) continue;
            if(mp[q.snake.h.y + d.y][q.snake.h.x + d.x] == '#' || mp[q.snake.h.y + d.y][q.snake.h.x + d.x] == '0') continue;
            for(size_t i {}; i < q.snake.b.size() - 1; ++i){   //  -1 because skip last body part (it moves)
                if(q.snake.b[i].x == q.snake.h.x + d.x && q.snake.b[i].y == q.snake.h.y + d.y){
                    valid = 0;
                }
            }
            if(q.snake.h.x + d.x == q.snake.b[1].x && q.snake.h.y + d.y == q.snake.b[1].y) continue;
            if(!valid) continue;

            double tempPw {};
            Snake tempSn {q.snake};
            bool tookPower {};
            if(mp[q.snake.h.y + d.y][q.snake.h.x + d.x] == 'P') tookPower = true;
            tempPw = q.points + calculatePoints(mp, tookPower, q.depth, powerDistanceMap.at(target)[q.snake.h.y + d.y][q.snake.h.x + d.x]);
            cerr << target.x << ' ' << target.y << '\n';
            tempSn.b = bodyMove(game, fullMap, q.snake.b, d, tookPower);
            tempSn.h.y = tempSn.b[0].y;
            tempSn.h.x = tempSn.b[0].x;
            if(q.origin.x == -1){
                que.push({tempSn, tempPw, tempSn.h, q.depth + 1});
            } else {
                que.push({tempSn, tempPw, q.origin, q.depth + 1});
            }
        }
    }

    return move;
}

void findPowerDistance(const Game & game, vector<vector<int>> & powerMap, Coord start, const vector<string> & field) {
    for (auto &row : powerMap) fill(row.begin(), row.end(), -1);

    struct Que {
        Coord p {};
        int d {};
    };
    queue<Que> que;
    que.push({start, 0});
    powerMap[start.y][start.x] = 0;

    const Coord dirs[4] = {{0, 1}, {1, 0}, {-1, 0}, {0, -1}};

    while(!que.empty()){
        Que q = que.front();
        que.pop();

        for(const auto& dir : dirs){
            int nx = q.p.x + dir.x;
            int ny = q.p.y + dir.y;
            if(nx < 0 || nx >= game.width || ny < 0 || ny >= game.height) continue;
            if(field[ny][nx] != '.') continue;
            if(powerMap[ny][nx] == -1) {
                powerMap[ny][nx] = q.d + 1;
                que.push({{nx, ny}, q.d + 1});
            }
        }
    }
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

    unordered_map<Coord, vector<vector<int>>> powerDistanceMap;

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

        powers.erase(std::remove_if(powers.begin(), powers.end(), [&](const Coord& p) {
            for (const auto & s : snake) {
                if (!s.mySnake) {
                    if (abs(p.x - s.h.x) + abs(p.y - s.h.y) == 1) {
                        return true; 
                    }
                }
            }
            return false;
        }), powers.end());

        //   1 sek bonus fiirst turn usage
        if(game.firstTurn){
            game.firstTurn = 0;
            for(auto& i : powers){
                powerDistanceMap[i].resize(game.height);
                for(auto& j : powerDistanceMap[i]){
                    j.resize(game.width);
                }
            }
            for(size_t i {}; i < powers.size(); ++i){
                findPowerDistance(game, powerDistanceMap[powers[i]], {powers[i].x, powers[i].y}, field);
            }
        }

        //  ----------  MAIN LOGIC
        vector<Coord> nextMoves {};
        for(auto & i : snake){
            if(!i.mySnake) continue;
            vector<string> mapSn {map};
            vector<string> fullMap {map};
            for(auto j : snake){
                if(i.id == j.id) continue;
                for(size_t z {}; z < j.b.size(); ++z){
                    if(j.b[z].x >= game.width || j.b[z].x < 0 || j.b[z].y >= game.height || j.b[z].y < 0) continue;
                    if(z != j.b.size() - 1) mapSn[j.b[z].y][j.b[z].x] = '0';
                    fullMap[j.b[z].y][j.b[z].x] = '0';
                }
            }
            for(auto i : nextMoves){
                if(i.x >= game.width || i.x < 0 || i.y >= game.height || i.y < 0) continue;
                mapSn[i.y][i.x] = '0';
                fullMap[i.y][i.x] = '0';
            }

            i.nextMove = simpleBfs(game, mapSn, fullMap, i, powerDistanceMap, powers, 5);
            
            if(i.nextMove == "UP"){
                nextMoves.push_back({i.h.x, i.h.y - 1});
            }
            if(i.nextMove == "DOWN"){
                nextMoves.push_back({i.h.x, i.h.y + 1});
            }
            if(i.nextMove == "LEFT"){
                nextMoves.push_back({i.h.x - 1, i.h.y});
            }
            if(i.nextMove == "RIGHT"){
                nextMoves.push_back({i.h.x + 1, i.h.y});
            }
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
    }
}
