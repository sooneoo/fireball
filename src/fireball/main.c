#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <math.h>


#define WIN_WIDTH 800
#define WIN_HEIGHT 600


typedef struct {
    float slop;
} LinearRegression;


float linear_regression_forward(LinearRegression * self, float x) {
    return (self->slop * x);
}


void linear_regression_backward(LinearRegression * self, size_t size, float * batch) {
    if(size > 1) {
        self->slop = (batch[size - 1] - batch[0]) / (float) (size - 1);
    }
}


#define PROJECTILE_BUFFER_SIZE 10
#define PROJECTILE_VELOCITY 1500


typedef struct {
    Vector2 position;
    Vector2 velocity;
    float radius;
} Projectile;


typedef struct {
    Projectile arr[PROJECTILE_BUFFER_SIZE];
    size_t head;
    size_t tail;
    size_t size;
} Projectile_Buff;


void projectile_buff_push(Projectile_Buff * self, Projectile projectile) {
    if(self->size < PROJECTILE_BUFFER_SIZE) {
        self->arr[self->head] = projectile;
        self->head = (self->head + 1) % PROJECTILE_BUFFER_SIZE;
        self->size ++;
    }
}


void projectile_buff_remove(Projectile_Buff * self, size_t index) {
    if(index < self->size) {
        for(size_t i = index; i < self->size - 1; i++) {
            int current_index = (self->tail + i) % PROJECTILE_BUFFER_SIZE;
            int next = (self->tail + i + 1) % PROJECTILE_BUFFER_SIZE;
            self->arr[current_index] = self->arr[next]; 
        }

        self->size --;
        if(self->head == 0) {
            self->head = PROJECTILE_BUFFER_SIZE - 1;
        } else {
            self->head --;
        }
    }
}


void projectile_buff_pop(Projectile_Buff * self) {
    if(self->size > 0) {
        self->tail = (self->tail + 1) % PROJECTILE_BUFFER_SIZE;
        self->size --;
    }
}


typedef struct {
    Vector2 position;
    float radius;
} Player;


#define PLAYER_VELOCITY 350


void player_draw(Player * self, float frame_time) {
    if(IsKeyDown(KEY_W) == true && self->position.y > 0) {
        self->position.y -= PLAYER_VELOCITY * frame_time;
    } else if(IsKeyDown(KEY_S) == true && self->position.y < WIN_HEIGHT) {
        self->position.y += PLAYER_VELOCITY * frame_time;
    }
    
    DrawCircleV(self->position, self->radius, GREEN);
}


typedef struct {
    Vector2 position;
    float radius;
    Projectile_Buff projectile_buff;
    LinearRegression regression;
    Vector2 player_pos_history;
    float fire_timer;


    bool use_regression;
} Tower;


float tower_target_aim(Tower * self, Vector2 player_position, float frame_time) {
    float a = self->position.x - player_position.x;
    float b = self->position.y - player_position.y;

    if(self->use_regression) {
        float time = sqrt(pow(a, 2) + pow(b, 2)) / (float) PROJECTILE_VELOCITY;
        float batch[] = {
            self->player_pos_history.y / frame_time
            , player_position.y / frame_time
        };

        linear_regression_backward(&self->regression, 2, batch);
        return atan((b - linear_regression_forward(&self->regression, time)) / a);
    } else { 
        return atan(b / a);
    }
}


void tower_projectile_fire(Tower * self, float target_angle) {
    Projectile projectile = (Projectile) {
        .position = self->position
        , .velocity = Vector2Scale((Vector2) {cos(target_angle), sin(target_angle)}, PROJECTILE_VELOCITY)
        , .radius = 5
    };

    projectile_buff_push(&self->projectile_buff, projectile);
}


void tower_draw(Tower * self, Vector2 player_position, float player_radius, float frame_time) {
    float target_angle = tower_target_aim(self, player_position, frame_time)  ;
    DrawPoly(self->position, 3, self->radius, 180 + (target_angle * RAD2DEG), RED);

    if(IsKeyPressed(KEY_R) == true) {
        self->use_regression = !self->use_regression;
    }
    
    if(self->fire_timer <= 0) {
        tower_projectile_fire(self, target_angle + PI);
        self->fire_timer = (float)((rand() + 400) % 800) / 1000.0f;
    } else {
        self->fire_timer -= frame_time;
    }

    if(self->use_regression == true) {
        DrawText("regression targeting: true", 10, 10, 30, BLACK);
    } else {
        DrawText("regression targeting: false", 10, 10, 30, BLACK);
    }
    
    for(size_t i = 0; i < self->projectile_buff.size;) {
        size_t index = (self->projectile_buff.tail + i) % PROJECTILE_BUFFER_SIZE;

        if(CheckCollisionCircles(player_position, player_radius
                    , self->projectile_buff.arr[index].position, self->projectile_buff.arr[index].radius) == true) {
            projectile_buff_remove(&self->projectile_buff, i);
        } else if(self->projectile_buff.arr[index].position.x < 0) {
            projectile_buff_pop(&self->projectile_buff);
        } else {
            self->projectile_buff.arr[index].position.x += self->projectile_buff.arr[index].velocity.x * frame_time;
            self->projectile_buff.arr[index].position.y += self->projectile_buff.arr[index].velocity.y * frame_time;
            
            DrawCircleV(self->projectile_buff.arr[index].position, self->projectile_buff.arr[index].radius, ORANGE);
            i++;
        }
    }

    self->player_pos_history = player_position;
}


int main(void) {
    InitWindow(WIN_WIDTH, WIN_HEIGHT, "Fireball");
 
    SetConfigFlags(FLAG_VSYNC_HINT);
    SetTargetFPS(144);

    Player player = {.position = {100, WIN_HEIGHT / 2}, .radius = 50};
    Tower tower = {.position = {WIN_WIDTH - 100, WIN_HEIGHT / 2}, .radius = 50};

    while(WindowShouldClose() == false) {
        float frame_time = GetFrameTime();

        BeginDrawing();
        ClearBackground(WHITE);

        player_draw(&player, frame_time);
        tower_draw(&tower, player.position, player.radius, frame_time);
       
        DrawFPS(WIN_WIDTH - 100, 10);
       
        EndDrawing();
    }

    CloseWindow();

    printf("program exit..\n");
    return EXIT_SUCCESS;
}


