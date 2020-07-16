#include "discord_game_sdk.h"
#include "zathura.h"

int main(){
for (;;) {
        DISCORD_REQUIRE(app.core->run_callbacks(app.core));
}
