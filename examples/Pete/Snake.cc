#include <random>
#include <deque>
#include <algorithm>

struct Pos {
  int x,y;

  bool operator==(const Pos&that) const {
    return x == that.x && y == that.y;
  }
};

class Snake {
  std::deque<Pos> body;
  bool dead;
  Pos food; // Maybe body[0] should be the food not the head?
  int BOARD_SIZE;

public:

  enum direction_t {
    Up, Down, Left, Right
  };

  enum content_t {
    Corpse, Body, Food, Empty, Head
  };

private:
  direction_t new_direction;
  direction_t direction;

  void spawn_food() {
    static std::uniform_int_distribution<int> randomLocationRange(0, BOARD_SIZE-1);
    static std::random_device rd;
    static std::mt19937 randomNumbers(rd());
    do {
      food.x = randomLocationRange( randomNumbers );
      food.y = randomLocationRange( randomNumbers );
    } while ( (std::find(body.begin(), body.end(), Pos{food.x,food.y}) != body.end() ));
  }

public:

  void setDirection( direction_t dir ) {
    if ( dir == Left && direction != Right) {
      new_direction = Left;
    }
    else if ( dir == Right && direction != Left) {
      new_direction = Right;
    }
    else if ( dir == Up && direction != Down) {
      new_direction = Up;
    }
    else if ( dir == Down && direction != Up){
      new_direction = Down;
    }
  }

  void reset() {
    dead=false;
    body.clear();
    body.push_front(Pos{ BOARD_SIZE / 2,  BOARD_SIZE / 2});
    direction=Left;
    new_direction=Left;
    spawn_food();
  }

  Snake( int board_size ) {
    BOARD_SIZE = board_size;
    reset();
  }

  bool pulse() {
    if (dead)
      return false;

    direction = new_direction;
    int head_x = body.front().x;
    int head_y = body.front().y;

    switch (direction) {
    case Left:
      head_x--; break;
    case Right:
      head_x++;break;
    case Up:
      head_y--;break;
    case Down:
      head_y++;break;
    }

    if (head_x < 0 || head_y < 0 || head_x == BOARD_SIZE || head_y == BOARD_SIZE) {
      dead = true;
    } else if ( std::find(body.begin(), body.end(), Pos{head_x,head_y}) != body.end() ) {
      dead = true;
    } else {
      body.push_front(Pos{head_x,head_y});
      if ( head_x == food.x && head_y == food.y) {
        spawn_food();
      } else {
        body.pop_back();
      }
    }
    return true;
  }

  //
  // Iteration logic
  //

  struct contents_t {
    int x, y;
    content_t content;
  };

  class iterator {
    Snake *parent;
    int index; // Maybe we could the the deque iterator internally.

  public:

    iterator( Snake* _parent) : parent{ _parent }, index( 0 ) {}

    bool operator!=(const iterator &that) const {
      return this->parent != that.parent || index != that.index;
    }

    iterator& operator++() {
      ++index;
      return *this;
    }

    contents_t operator*() const {
      if (index == 0) {
        return contents_t{parent->food.x, parent->food.y, Food};
      } else if (index == 1 && !parent->dead) {
        return contents_t{parent->body[0].x, parent->body[0].y, Head};
      } else {
        if (parent->dead)
          return contents_t{parent->body[index - 1].x, parent->body[index - 1].y, Corpse};
        else
          return contents_t{parent->body[index - 1].x, parent->body[index - 1].y, Body};
      }
    }

    friend Snake;
  };

  iterator begin() {
    return iterator(this);
  }

  iterator end() {
    auto i = iterator(this);
    i.index = body.size() + 1;
    return i;
  }

};


const unsigned int TILE_SIZE = 20;
const unsigned int BOARD_SIZE = 40;
const unsigned int SCREEN_WIDTH  = (2 + BOARD_SIZE) * TILE_SIZE;
const unsigned int SCREEN_HEIGHT = (2 + BOARD_SIZE) * TILE_SIZE;
Snake snake(BOARD_SIZE);

void setup() {
  size(SCREEN_WIDTH, SCREEN_HEIGHT);
  noStroke();
  frameRate(20);
  background(BLUE);
}

void draw() {
  fill(0);
  rect(1*TILE_SIZE, 1*TILE_SIZE, TILE_SIZE*BOARD_SIZE,TILE_SIZE*BOARD_SIZE);

  for (auto&& [ x, y, content ] : snake) {
    switch (content) {
    default:
      fill(BLACK);
      break;
    case Snake::Food:
      fill(RED);
      break;
    case Snake::Body:
      fill(WHITE);
      break;
    case Snake::Corpse:
      fill(YELLOW);
      break;
    case Snake::Head:
      fill(GREEN);
      break;
    }
    rect((x+1)*TILE_SIZE, (y+1)*TILE_SIZE, TILE_SIZE, TILE_SIZE);
  }
  snake.pulse();
}

void keyPressed() {
  switch (keyCode) {
  case ' ':
    snake.reset();
    break;
  case LEFT:
    snake.setDirection( Snake::Left );
    break;
  case RIGHT:
    snake.setDirection( Snake::Right );
    break;
  case UP:
    snake.setDirection( Snake::Up );
    break;
  case DOWN:
    snake.setDirection( Snake::Down );
  };
}
