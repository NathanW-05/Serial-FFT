#ifndef UI_H
#define UI_H

const char* speed_text(float speed);
const char* status_text();

void init_raylib();
void render_loop();
int should_return();

#endif