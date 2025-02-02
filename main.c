#include <stdlib.h>
#include <stdio.h>

#include "raylib.h"
#include "raymath.h"

#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 500

#define ENABLE_GAMEPAD 1

const float SPEED = 150;
const float DEADZONE = 0.1;
const float ZOOM_MAX = 4.0;

typedef struct SnakePart {
  struct SnakePart* next;
  Vector2 pos;
  float length;
} SnakePart;

typedef struct {
  SnakePart* head;
  SnakePart* tail;
  float thickness;
} SnakeQueue;

void addSnakeFront(SnakeQueue* self, Vector2 pos, float length) {
  SnakePart* new = malloc(sizeof(SnakePart));
  new->pos = pos;
  new->length = length;
  new->next = NULL;

  if (self->head == NULL) {
    self->head = new;
    self->tail = new;
    return;
  }

  self->head->next = new;
  self->head = new;
}

void moveSnake(SnakeQueue* self, Vector2 direction, float dt) {
  for (SnakePart* part = self->tail; part != NULL; part = part->next){
    if (part->next == NULL) {
      part->pos = Vector2Add(part->pos, Vector2Scale(direction, SPEED * dt));
    }
    else {
      Vector2 dpos = Vector2Subtract(part->next->pos, part->pos);
      if ( Vector2Length(dpos) > part->length) {
        part->pos = Vector2Add(
            part->pos,
            Vector2Scale(
              dpos,
              dt * SPEED
            )
        );
      }
    }
  };
}

int main(){
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Snake");

  // Load the shader
  Shader shader = LoadShader("background.vs", "background.fs");

  // Get the uniform locations
  int mvpLoc = GetShaderLocation(shader, "mvp");
  int resolutionLoc = GetShaderLocation(shader, "screenSize");
  int cameraTransformLoc = GetShaderLocation(shader, "cameraTransform");

  float time = 0.0f;


  int gamepad = 0;

  Camera2D camera = {
    .zoom = 1.0,
    .offset = {0, 0},
    .rotation = 0,
    .target = {0, 0},
  };

  SnakeQueue snake = {
    .head = NULL,
    .tail = NULL,
    .thickness = 10
  };
  int partCount = 300;
  float initialLength = 300;
  for (int i = 0; i < partCount; i++) {
    addSnakeFront(
          &snake
        , (Vector2){250, 100 + initialLength * i / partCount}
        , initialLength / partCount
    );
  }

  Vector2 velocity = {0, 0};
  Vector2 input_direction = {0, 0};
  Vector2 real_direction = {0, 0};
  Vector2 look_direction = {0, 0}; 
  

  while(!WindowShouldClose()){
    float dt = GetFrameTime();
    time += dt;

    Vector2 resolution = { GetScreenWidth(), GetScreenHeight() };
    SetShaderValue(shader, resolutionLoc, &resolution, SHADER_UNIFORM_VEC2);
    SetShaderValueMatrix(shader, mvpLoc, GetCameraMatrix2D(camera));
    SetShaderValueMatrix(shader, cameraTransformLoc, GetCameraMatrix2D(camera));
    
    while (
      !IsGamepadAvailable(gamepad) && 
      gamepad > 0
    ) gamepad--;

    while (
      IsGamepadAvailable(gamepad) && 
      TextFindIndex(GetGamepadName(gamepad), "Touchpad") > -1
    ) gamepad++;

    if (
     IsGamepadAvailable(gamepad)
    ) {
      printf("%s\n", GetGamepadName(gamepad));

      float targetZoom = 
        1.0 +
        (GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_TRIGGER) + 1) / 2 *
        ZOOM_MAX;
      camera.zoom = Lerp(camera.zoom, targetZoom, dt / 0.2);


      Vector2 temp_direction = (Vector2){
        GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X),
        GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y)
      };

      look_direction = (Vector2){
        GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X),
        GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y)
      };

      if (Vector2Length(temp_direction) > DEADZONE) {
        input_direction = Vector2Normalize(temp_direction);
      }

      real_direction = Vector2Normalize(
        Vector2Lerp(
          real_direction,
          input_direction,
          dt * 20
        )
      );

    }
    {
      float targetZoom = IsKeyDown(KEY_SPACE) ? ZOOM_MAX : 1.0;
      camera.zoom = Lerp(camera.zoom, targetZoom, dt / 0.2);

      Vector2 temp_direction = Vector2Normalize(
        Vector2Subtract(
          GetMousePosition(),
          (Vector2){GetRenderWidth()/2,GetRenderHeight()/2}
        )
      );

      if (Vector2Length(temp_direction) > DEADZONE) {
        input_direction = Vector2Normalize(temp_direction);
      }

      real_direction = Vector2Normalize(
        Vector2Lerp(
          real_direction,
          input_direction,
          dt * 20
        )
      );

      look_direction = (Vector2){0, 0};
      if (IsKeyDown(KEY_W)) look_direction = Vector2Add(
          look_direction, (Vector2){0, -1}
        );
      if (IsKeyDown(KEY_A)) look_direction = Vector2Add(
          look_direction, (Vector2){-1, 0}
        );
      if (IsKeyDown(KEY_S)) look_direction = Vector2Add(
          look_direction, (Vector2){0, 1}
        );
      if (IsKeyDown(KEY_D)) look_direction = Vector2Add(
          look_direction, (Vector2){1, 0}
        );

      if (Vector2Length(look_direction) > DEADZONE)
        look_direction = Vector2Normalize(look_direction);

    }

    moveSnake(&snake, real_direction, dt);

    Vector2 sdiv2 = {GetRenderWidth()/2,GetRenderHeight()/2};
    camera.target = Vector2Lerp(
        camera.target,
        Vector2Add(
          Vector2Subtract(
            snake.head->pos,
            sdiv2
          ),
          Vector2Scale(
            look_direction,
            200 / camera.zoom
          )
        ),
        dt * 5
    );
    camera.offset = Vector2Scale(sdiv2, 1 - camera.zoom);
    

    BeginDrawing();

    ClearBackground((Color){7,5,13,255});

    BeginShaderMode(shader);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);
    EndShaderMode();
    
    BeginMode2D(camera);
    
    for (float x = 0; x < GetRenderWidth(); x += 50 * camera.zoom) {
    
    };


    int i = 0;
    Color colors[] =
      {
        {250, .a = 255},
        {250, .a = 255},
        {250, .a = 255},
        WHITE,
        {250, .a = 255},
        {250, .a = 255},
        {250, .a = 255},
      };

    for (SnakePart* part = snake.tail; part != NULL; part = part->next) {
      Vector2 pos = part->pos;
      
      if (part->next != NULL) {
        DrawCircleV(pos, snake.thickness, colors[i % 7]);
      }
      else {
        DrawCircleV(pos, snake.thickness * 1.2, colors[i % 7]);
      }
      i += 4;
    }

    DrawLineEx(
        Vector2Add(
          snake.head->pos,
          Vector2Scale(real_direction, snake.thickness * 2)
        ),
        Vector2Add(
          snake.head->pos,
          Vector2Scale(real_direction, snake.thickness * 6)
        ),
        5,
        WHITE
    );
    EndDrawing();
  }
  return 0;
}
