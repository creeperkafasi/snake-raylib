#include <stdlib.h>
#include <stdio.h>

#include "raylib.h"
#include "raymath.h"

#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 500

#define ENABLE_GAMEPAD 1

const float SPEED = 150;
const float BOOST = 4;
const float DEADZONE = 0.1;
const float ZOOM_MAX = 2.0;

typedef struct SnakePart {
  struct SnakePart* next;
  Vector2 pos;
  float length;
} SnakePart;

typedef struct {
  SnakePart* head;
  SnakePart* tail;
  float thickness;
  int length;
  Vector2 movement_direction;
  Vector2 look_direction;
  float boost_time;
} Snake;

void addSnakeFront(Snake* self, Vector2 pos, float length) {
  SnakePart* new = malloc(sizeof(SnakePart));
  new->pos = pos;
  new->length = length;
  new->next = NULL;

  self->length++;

  if (self->head == NULL) {
    self->head = new;
    self->tail = new;
    return;
  }

  self->head->next = new;
  self->head = new;
}

void addSnakeTail(Snake* self, Vector2 pos, float length) {
  SnakePart* new = malloc(sizeof(SnakePart));
  new->pos = pos;
  new->length = length;

  new->next = self->tail;
  self->tail = new;
  self->length++;
}


void popSnakeTail(Snake* self) {
  SnakePart* temp = self->tail; 
  self->tail = self->tail->next;
  free(temp);
  self->length--;
}

void moveSnake(Snake* self, float dt) {
  float speed = SPEED;
  if (self->boost_time > 0) {
    speed *= BOOST;
    self->boost_time -= dt;
  }
  for (SnakePart* part = self->tail; part != NULL; part = part->next){
    if (part->next == NULL) {
      part->pos = Vector2Add(part->pos, Vector2Scale(self->movement_direction, speed * dt));
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

void renderSnake(Snake* snake) {
  int i = snake->length - 1;
  Color colors[] =
    {
      {250, .a = 255},
      {250, .a = 255},
      {250, .a = 255},
      {250, .a = 255},
      {250, .a = 255},
      {250, .a = 255},
      {250, .a = 255},
      {250, .a = 255},
      WHITE,
    };
  int colorCount = sizeof(colors) / sizeof(Color);

  for (SnakePart* part = snake->tail; part != NULL; part = part->next) {
    Vector2 pos = part->pos;
    
    int colorIndex = i % colorCount;

    if (part->next != NULL) {
      DrawCircleV(pos, snake->thickness, colors[colorIndex]);
    }
    else {
      // Head
      DrawCircleV(pos, snake->thickness * 1.2, colors[colorIndex]);

      // Right Eye
      DrawCircleV(
          Vector2Add(
            Vector2Add(
              pos,
              Vector2Scale(snake->movement_direction, snake->thickness * 0.6)
            ),
            Vector2Scale(
              Vector2Rotate(snake->movement_direction, PI/2),
              snake->thickness * 0.5
            )
          ),
          snake->thickness * 0.4, WHITE);
      DrawCircleV(
          Vector2Add(
            Vector2Add(
              pos,
              Vector2Add(
                Vector2Scale(
                  snake->movement_direction,
                  snake->thickness * 0.6
                ),
                Vector2Scale(
                  snake->look_direction,
                  snake->thickness * 0.2
                )
              )
            ),
            Vector2Scale(
              Vector2Rotate(snake->movement_direction, PI/2),
              snake->thickness * 0.5
            )
          ),
          snake->thickness * 0.2, BLACK);

      // Left Eye
      DrawCircleV(
          Vector2Add(
            Vector2Add(
              pos,
              Vector2Scale(snake->movement_direction, snake->thickness * 0.6)
            ),
            Vector2Scale(
              Vector2Rotate(snake->movement_direction, PI/2),
              snake->thickness * -0.5
            )
          ),
          snake->thickness * 0.4, WHITE);
      DrawCircleV(
          Vector2Add(
            Vector2Add(
              pos,
              Vector2Add(
                Vector2Scale(
                  snake->movement_direction,
                  snake->thickness * 0.6
                ),
                Vector2Scale(
                  snake->look_direction,
                  snake->thickness * 0.2
                )
              )
            ),
            Vector2Scale(
              Vector2Rotate(snake->movement_direction, PI/2),
              snake->thickness * -0.5
            )
          ),
          snake->thickness * 0.2, BLACK);


    }
    i--;
  }


  float triangle_size = 0.8;
  float triangle_offset = 3;
  DrawTriangle(
      Vector2Add(
        Vector2Add(
          snake->head->pos,
          Vector2Scale(snake->movement_direction, snake->thickness * triangle_offset)
        ),
        Vector2Rotate(
          Vector2Scale(snake->movement_direction, snake->thickness * triangle_size),
          -PI/2
        )
      ),
      Vector2Add(
        Vector2Add(
          snake->head->pos,
          Vector2Scale(snake->movement_direction, snake->thickness * triangle_offset)
        ),
        Vector2Rotate(
          Vector2Scale(snake->movement_direction, snake->thickness * triangle_size),
          PI/2
        )
      ),
      Vector2Add(
        snake->head->pos,
        Vector2Scale(snake->movement_direction, snake->thickness * (triangle_offset + triangle_size))
      ),
      (Color){255,255,255,100}
  );
}

int gamepad = 0;

int screen_mainmenu() {
    while (
      !IsGamepadAvailable(gamepad) && 
      gamepad > 0
    ) gamepad--;

    while (
      IsGamepadAvailable(gamepad) && 
      TextFindIndex(GetGamepadName(gamepad), "Touchpad") > -1
    ) gamepad++;

    if (IsGamepadAvailable(gamepad)){
      if (IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) return 1;
    }
    else {
      if (IsKeyPressed(KEY_ENTER)) return 1;
    }

    BeginDrawing();
    ClearBackground(BLACK);

    if (IsGamepadAvailable(gamepad)){
      DrawTextEx(
        GetFontDefault(),
        "Press A\n"
        "to start",
        (Vector2){50,150}, 36, 10, WHITE
      );
    }
    else {
      DrawTextEx(
        GetFontDefault(),
        "Press ENTER\n"
        "to start",
        (Vector2){50,150}, 36, 10, WHITE
      );
    }

    EndDrawing();
    return 0;
}

void input_gamepad(Camera2D* camera, Snake* snake, float dt){
  float targetZoom = 
    1.0 +
    (GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_TRIGGER) + 1) / 2 *
    ZOOM_MAX;
  camera->zoom = Lerp(camera->zoom, targetZoom, dt / 0.2);


  Vector2 temp_direction = (Vector2){
    GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X),
    GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y)
  };

  snake->look_direction = (Vector2){
    GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X),
    GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y)
  };

  if (Vector2Length(temp_direction) > DEADZONE) {
   snake->movement_direction = Vector2Normalize(
      Vector2Lerp(
        snake->movement_direction,
        Vector2Normalize(temp_direction),
        dt / 0.05
      )
    );
  }

  if (IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
    for (int i = 0; i < 10; i++) {
      addSnakeTail(snake, snake->tail->pos, snake->tail->length);
    }
  }

  
  if ( IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) &&
       (snake->length > 10) ) {
    //printf("boost %f && %d\n", snake->boost_time, snake->length);
    if ( (snake->boost_time <= 0) && (snake->length > 10) ) {
      snake->boost_time += 0.2;
      popSnakeTail(snake);
    }
  }
  

  //real_direction = input_direction;
  
}

void input_keymouse(Camera2D* camera, Snake* snake, float dt) {
      float targetZoom = IsKeyDown(KEY_SPACE) ? ZOOM_MAX : 1.0;
      camera->zoom = Lerp(camera->zoom, targetZoom, dt / 0.2);

      Vector2 temp_direction = Vector2Normalize(
        Vector2Subtract(
          GetMousePosition(),
          (Vector2){GetRenderWidth()/2,GetRenderHeight()/2}
        )
      );

      if (Vector2Length(temp_direction) > DEADZONE) {
        snake->movement_direction = Vector2Normalize(
          Vector2Lerp(
            snake->movement_direction,
            temp_direction,
            dt * 20
          )
        );
      }

      snake->look_direction = (Vector2){0, 0};
      if (IsKeyDown(KEY_W)) snake->look_direction = Vector2Add(
          snake->look_direction, (Vector2){0, -1}
        );
      if (IsKeyDown(KEY_A)) snake->look_direction = Vector2Add(
          snake->look_direction, (Vector2){-1, 0}
        );
      if (IsKeyDown(KEY_S)) snake->look_direction = Vector2Add(
          snake->look_direction, (Vector2){0, 1}
        );
      if (IsKeyDown(KEY_D)) snake->look_direction = Vector2Add(
          snake->look_direction, (Vector2){1, 0}
        );

      if (Vector2Length(snake->look_direction) > DEADZONE)
        snake->look_direction = Vector2Normalize(snake->look_direction);
}

int main(){
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Snake");

  // Load the shader
  Shader background_shader = LoadShader("background.vs", "background.fs");

  // Get the uniform locations
  int mvpLoc = GetShaderLocation(background_shader, "mvp");
  int resolutionLoc = GetShaderLocation(background_shader, "screenSize");
  int cameraTransformLoc = GetShaderLocation(background_shader, "cameraTransform");


  Shader speedlines_shader = LoadShader(0, "speedlines.fs");
  
  // Get uniform locations
  int iResolutionLoc = GetShaderLocation(speedlines_shader, "iResolution");
  int iTimeLoc = GetShaderLocation(speedlines_shader, "iTime");

  float time = 0.0f;


  int gamepad = 0;

  Camera2D camera = {
    .zoom = 1.0,
    .offset = {0, 0},
    .rotation = 0,
    .target = {0, 0},
  };

  Snake snake = {
    .head = NULL,
    .tail = NULL,
    .thickness = 10,
    .length = 0,
    .look_direction = {0, 0},
    .movement_direction = {1, 0},
    .boost_time = 0,
  };
  int partCount = 50;
  float initialLength = 100;
  for (int i = 0; i < partCount; i++) {
    addSnakeFront(
          &snake
        , (Vector2){250, 100 + initialLength * i / partCount}
        , initialLength / partCount
    );
  }

  Vector2 velocity = {0, 0};
  Vector2 input_direction = {0, 0};

  while(!WindowShouldClose()){
    if (screen_mainmenu()) break;
  }
  while(!WindowShouldClose()){
    float dt = GetFrameTime();
    time += dt;

    
    while (
      !IsGamepadAvailable(gamepad) && 
      gamepad > 0
    ) gamepad--;

    while (
      IsGamepadAvailable(gamepad) && 
      TextFindIndex(GetGamepadName(gamepad), "Touchpad") > -1
    ) gamepad++;

    if ( IsGamepadAvailable(gamepad) ) {
      input_gamepad(&camera, &snake, dt);
    }
    else {
      input_keymouse(&camera, &snake, dt);
    }

    moveSnake(&snake, dt);

    Vector2 sdiv2 = {GetRenderWidth()/2,GetRenderHeight()/2};
    camera.target = Vector2Lerp(
        camera.target,
        Vector2Add(
          Vector2Subtract(
            snake.head->pos,
            sdiv2
          ),
          Vector2Scale(
            snake.look_direction,
            200 / camera.zoom
          )
        ),
        dt / 0.1
    );
    camera.offset = Vector2Scale(sdiv2, 1 - camera.zoom);
    

    BeginDrawing();

    // Draw background
    ClearBackground((Color){7,5,13,255});

    Vector2 resolution = { GetScreenWidth(), GetScreenHeight() };
    SetShaderValue(background_shader, resolutionLoc, &resolution, SHADER_UNIFORM_VEC2);
    SetShaderValueMatrix(background_shader, mvpLoc, GetCameraMatrix2D(camera));
    SetShaderValueMatrix(background_shader, cameraTransformLoc, GetCameraMatrix2D(camera));

    BeginShaderMode(background_shader);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
    EndShaderMode();

    
    BeginMode2D(camera);

    renderSnake(&snake);

    EndMode2D();

    // Speedlines during boost
    if (snake.boost_time > 0) {
      SetShaderValue(
          speedlines_shader,
          iResolutionLoc,
          &(Vector2){GetScreenWidth(),GetScreenHeight()},
          SHADER_UNIFORM_VEC2
      );
      SetShaderValue(speedlines_shader, iTimeLoc, &time, SHADER_UNIFORM_FLOAT);
      BeginShaderMode(speedlines_shader);
      DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
      EndShaderMode();
    }

    EndDrawing();
  }
  return 0;
}
