#include "ItemManager.h"
#include "player.h"
#include <cmath>

SDL_Surface* ItemManager::itemPowerSmallImage = NULL;
SDL_Surface* ItemManager::itemPointImage = NULL;
SDL_Surface* ItemManager::itemPowerBigImage = NULL;
SDL_Surface* ItemManager::itemBombImage = NULL;
SDL_Surface* ItemManager::itemFullPowerImage = NULL;
SDL_Surface* ItemManager::itemLifeImage = NULL;
SDL_Surface* ItemManager::itemPointBulletImage = NULL;

int ItemManager::score = 0;
int ItemManager::lives = 2;
int ItemManager::bombs = 3;
int ItemManager::powerItemCountForScore = 0;

const float ItemManager::STATE2_DURATION = 1.0f;
const float ItemManager::MAGNET_SPEED   = 480.0f;   // 8 px/frame * 60
const float ItemManager::GRAVITY        = 108.0f;   // 0.03 px/frame² * 60 * 60
const float ItemManager::MAX_FALL_SPEED = 180.0f;   // 3 px/frame * 60
const float ItemManager::UPWARD_DRIFT   = -132.0f;  // -2.2 px/frame * 60
const float ItemManager::COLLECT_RADIUS = 16.0f;

const int ItemManager::POWER_UP_THRESHOLDS[11] = {
    8, 16, 32, 48, 64, 80, 96, 128, 999, 1, 0
};

const int ItemManager::POWER_ITEM_SCORE[31] = {
    10, 20, 30, 40, 50, 60, 70, 80, 90, 100,
    200, 300, 400, 500, 600, 700, 800, 900, 1000,
    2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000,
    10000, 11000, 12000, 51200
};

ItemManager::ItemManager() : nextItemIndex(0), playerPtr(NULL) {
    if (itemPowerSmallImage == NULL)
        itemPowerSmallImage = load_sprite("res/etama/etama2.png", 0, 64, 16, 16, 16.0, 16.0);
    if (itemPointImage == NULL)
        itemPointImage = load_sprite("res/etama/etama2.png", 16, 64, 16, 16, 16.0, 16.0);
    if (itemPowerBigImage == NULL)
        itemPowerBigImage = load_sprite("res/etama/etama2.png", 32, 64, 16, 16, 16.0, 16.0);
    if (itemBombImage == NULL)
        itemBombImage = load_sprite("res/etama/etama2.png", 48, 64, 16, 16, 16.0, 16.0);
    if (itemFullPowerImage == NULL)
        itemFullPowerImage = load_sprite("res/etama/etama2.png", 64, 64, 16, 16, 16.0, 16.0);
    if (itemLifeImage == NULL)
        itemLifeImage = load_sprite("res/etama/etama2.png", 80, 64, 16, 16, 16.0, 16.0);
    if (itemPointBulletImage == NULL)
        itemPointBulletImage = load_sprite("res/etama/etama2.png", 96, 64, 16, 16, 16.0, 16.0);

    for (int i = 0; i < MAX_ITEMS; i++)
        items[i].isActive = false;
}

ItemManager::~ItemManager() {}

void ItemManager::set_player(Player* p) {
    playerPtr = p;
}

SDL_Surface* ItemManager::get_item_surface(int itemType) {
    switch (itemType) {
        case ITEM_POWER_SMALL:  return itemPowerSmallImage;
        case ITEM_POINT:        return itemPointImage;
        case ITEM_POWER_BIG:    return itemPowerBigImage;
        case ITEM_BOMB:         return itemBombImage;
        case ITEM_FULL_POWER:   return itemFullPowerImage;
        case ITEM_LIFE:         return itemLifeImage;
        case ITEM_POINT_BULLET: return itemPointBulletImage;
        default:                return itemPointImage;
    }
}

void ItemManager::spawn_item(float x, float y, int itemType, int state) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        int idx = (nextItemIndex + i) % MAX_ITEMS;
        if (!items[idx].isActive) {
            Item* it = &items[idx];
            it->x = x;
            it->y = y;
            it->type = itemType;
            it->speedX = 0.0f;
            it->speedY = UPWARD_DRIFT;
            it->isActive = true;
            it->state = state;
            it->timer = 0.0f;
            it->startX = x;
            it->startY = y;

            if (state == 2) {
                // Random target position (th06: x:[48,336], y:[-64,128])
                it->targetX = x + (rand() % 289) - 144;  // x ±144
                it->targetY = y + (rand() % 193) - 128;  // y -128..+64
                if (it->targetX < 48)  it->targetX = 48;
                if (it->targetX > 336) it->targetX = 336;
                if (it->targetY < -64) it->targetY = -64;
                if (it->targetY > 128) it->targetY = 128;
            }

            nextItemIndex = (idx + 1) % MAX_ITEMS;
            return;
        }
    }
}

void ItemManager::update(float dt) {
    if (playerPtr == NULL) return;

    PlayerPosition playerPos = playerPtr->get_player_position();
    float px = playerPos.x;
    float py = playerPos.y;
    int currentPower = Player::get_power();
    bool autoCollect = (currentPower >= MAX_POWER && py < 128);

    for (int i = 0; i < MAX_ITEMS; i++) {
        Item* it = &items[i];
        if (!it->isActive) continue;

        if (it->state == 2) {
            it->timer += dt;
            if (it->timer < STATE2_DURATION) {
                float t = it->timer / STATE2_DURATION;
                it->x = it->targetX * t + it->startX * (1.0f - t);
                it->y = it->targetY * t + it->startY * (1.0f - t);
            } else {
                it->state = 0;
                it->speedX = 0.0f;
                it->speedY = UPWARD_DRIFT;
            }
        }

        // proximity magnet: items within 100px of player center auto-collect
        float pdx = px - it->x;
        float pdy = py - it->y;
        float pdist = sqrtf(pdx * pdx + pdy * pdy);
        bool nearPlayer = (pdist < 100.0f);

        if (it->state == 1 || (it->state == 0 && (autoCollect || nearPlayer))) {
            it->state = 1;
            if (pdist > 1.0f) {
                it->x += (pdx / pdist) * MAGNET_SPEED * dt;
                it->y += (pdy / pdist) * MAGNET_SPEED * dt;
            }
        }

        if (it->state == 0) {
            it->speedY += GRAVITY * dt;
            if (it->speedY > MAX_FALL_SPEED)  it->speedY = MAX_FALL_SPEED;
            if (it->speedY < UPWARD_DRIFT)     it->speedY = UPWARD_DRIFT;
            it->x += it->speedX * dt;
            it->y += it->speedY * dt;
        }

        if (it->y > 680) {
            it->isActive = false;
            continue;
        }

        float dx = px - it->x;
        float dy = py - it->y;
        if (fabsf(dx) < COLLECT_RADIUS && fabsf(dy) < COLLECT_RADIUS) {
            it->isActive = false;

            switch (it->type) {
                case ITEM_POWER_SMALL: {
                    if (currentPower >= MAX_POWER) {
                        int idx = powerItemCountForScore;
                        if (idx < 31) score += POWER_ITEM_SCORE[idx];
                        powerItemCountForScore++;
                        if (powerItemCountForScore > 30)
                            powerItemCountForScore = 30;
                    } else {
                        Player::set_power(Player::get_power() + 1);
                        score += 10;
                        for (int t = 0; t < 11; t++) {
                            if (Player::get_power() == POWER_UP_THRESHOLDS[t]) {
                                break;
                            }
                        }
                        if (Player::get_power() >= MAX_POWER) {
                            Player::set_power(MAX_POWER);
                            // TODO: TurnAllBulletsIntoPoints()
                        }
                    }
                    break;
                }

                case ITEM_POWER_BIG: {
                    if (currentPower >= MAX_POWER) {
                        for (int n = 0; n < 8; n++) {
                            int idx = powerItemCountForScore;
                            if (idx < 31) score += POWER_ITEM_SCORE[idx];
                            powerItemCountForScore++;
                            if (powerItemCountForScore > 30)
                                powerItemCountForScore = 30;
                        }
                    } else {
                        int gains = 8;
                        while (gains-- > 0) {
                            if (Player::get_power() >= MAX_POWER) {
                                int idx = powerItemCountForScore;
                                if (idx < 31) score += POWER_ITEM_SCORE[idx];
                                powerItemCountForScore++;
                                break;
                            }
                            Player::set_power(Player::get_power() + 1);
                            score += 10;
                            for (int t = 0; t < 11; t++) {
                                if (Player::get_power() == POWER_UP_THRESHOLDS[t]) {
                                    // TODO: power-up popup
                                    break;
                                }
                            }
                        }
                        if (Player::get_power() >= MAX_POWER) {
                            Player::set_power(MAX_POWER);
                            // TODO: TurnAllBulletsIntoPoints()
                        }
                    }
                    break;
                }

                case ITEM_POINT: {
                    // Score based on Y position (th06 Normal: max 100000 at top, min 60000 at bottom)
                    if ((int)it->y < 128) {
                        score += 100000;
                    } else {
                        score += 60000 - (((int)it->y - 128) * 100);
                        if (score < 0) score += 100000; // clamp hack
                    }
                    break;
                }

                case ITEM_BOMB: {
                    if (bombs < 8) bombs++;
                    break;
                }

                case ITEM_LIFE: {
                    if (lives < 8) lives++;
                    // TODO: play 1UP sound
                    break;
                }

                case ITEM_FULL_POWER: {
                    Player::set_power(MAX_POWER);
                    score += 1000;
                    // TODO: TurnAllBulletsIntoPoints()
                    // TODO: power-up popup
                    break;
                }

                case ITEM_POINT_BULLET: {
                    // Simplified: flat 500 score (th06 adds graze bonus)
                    score += 500;
                    break;
                }
            }
        }
    }
}

void ItemManager::render() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        Item* it = &items[i];
        if (!it->isActive) continue;

        SDL_Surface* sprite = get_item_surface(it->type);
        if (sprite == NULL) continue;

        int drawX = (int)(it->x - sprite->w / 2);
        int drawY = (int)(it->y - sprite->h / 2);

        // Clamp indicator at top of screen when item is above visible area
        if (drawY < -8) {
            drawY = 8;
            // TODO: use alternate sprite for off-screen indicator
        }

        apply_surface(drawX, drawY, sprite, screen);
    }
}

void ItemManager::clear_all() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (items[i].isActive && items[i].state == 0) {
            items[i].state = 1;
        }
    }
}
