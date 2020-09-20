#include <stdio.h>
#include <stdlib.h>
#include "nav.h"
#include "knobs.h"

nav::nav(int xPos, int yPos, int size) : instrument(xPos, yPos, size)
{
    setName("Nav");
    addVars();
    simVars = &globals.simVars->simVars;

#ifndef _WIN32
    // Only have hardware knobs on Raspberry Pi
    if (globals.hardwareKnobs) {
        addKnobs();
    }
#endif

    resize();
}

/// <summary>
/// Destroy and recreate all bitmaps as instrument has been resized
/// </summary>
void nav::resize()
{
    destroyBitmaps();

    // Create bitmaps scaled to correct size (original size is 800)
    scaleFactor = size / 1600.0f;

    // 0 = Original (loaded) bitmap
    ALLEGRO_BITMAP* orig = loadBitmap("nav.png");
    addBitmap(orig);

    if (bitmaps[0] == NULL) {
        return;
    }

    // 1 = Destination bitmap (all other bitmaps get assembled to here)
    ALLEGRO_BITMAP* bmp = al_create_bitmap(size, size / 4);
    addBitmap(bmp);

    // 2 = Main Nav
    bmp = al_create_bitmap(size, size / 4);
    al_set_target_bitmap(bmp);
    al_draw_scaled_bitmap(orig, 0, 0, 1600, 400, 0, 0, size, size / 4, 0);
    addBitmap(bmp);

    // 3 = Main Autopilot
    bmp = al_create_bitmap(size, size / 4);
    al_set_target_bitmap(bmp);
    al_draw_scaled_bitmap(orig, 0, 400, 1600, 400, 0, 0, size, size / 4, 0);
    addBitmap(bmp);

    // 4 = Digits
    bmp = al_create_bitmap(380, 80);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 0, 800, 380, 80, 0, 0, 0);
    addBitmap(bmp);

    // 5 = Dot
    bmp = al_create_bitmap(20, 80);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 380, 800, 20, 80, 0, 0, 0);
    addBitmap(bmp);

    // 6 = Switch
    bmp = al_create_bitmap(80, 34);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 400, 800, 80, 34, 0, 0, 0);
    addBitmap(bmp);

    // 7 = Transponder state selected
    bmp = al_create_bitmap(320, 34);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 480, 800, 320, 34, 0, 0, 0);
    addBitmap(bmp);

    // 8 = Transponder state
    bmp = al_create_bitmap(320, 34);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 480, 834, 320, 34, 0, 0, 0);
    addBitmap(bmp);

    // 9 = Autopilot switches
    bmp = al_create_bitmap(400, 34);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 800, 800, 400, 34, 0, 0, 0);
    addBitmap(bmp);

    // 10 = Autopilot display
    bmp = al_create_bitmap(1024, 50);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 0, 880, 1024, 50, 0, 0, 0);
    addBitmap(bmp);

    // 11 = Autopilot vertical speed digits
    bmp = al_create_bitmap(320, 50);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 1024, 880, 320, 50, 0, 0, 0);
    addBitmap(bmp);

    // 12 = Autopilot vertical speed fpm
    bmp = al_create_bitmap(162, 50);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 1344, 880, 162, 50, 0, 0, 0);
    addBitmap(bmp);

    // 13 = Autopilot vertical speed minus
    bmp = al_create_bitmap(23, 50);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 1506, 880, 23, 50, 0, 0, 0);
    addBitmap(bmp);

    al_set_target_backbuffer(globals.display);
}

/// <summary>
/// Draw the instrument at the stored position
/// </summary>
void nav::render()
{
    if (bitmaps[0] == NULL) {
        return;
    }

    // Use normal blender
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    // Draw stuff into dest bitmap
    al_set_target_bitmap(bitmaps[1]);

    if (switchSel < 6) {
        renderNav();
    }
    else {
        renderAutopilot();
    }

    // Position dest bitmap on screen
    al_set_target_backbuffer(globals.display);
    al_draw_bitmap(bitmaps[1], xPos, yPos, 0);

    if (!globals.active) {
        dimInstrument();
    }
}

/// <summary>
/// Draw the Nav Panel at the stored position
/// </summary>
void nav::renderNav()
{
    // Add main nav
    al_draw_bitmap(bitmaps[2], 0, 0, 0);

    // Add panel 1 frequencies
    addFreq3dp(com1Freq, 237, 19);
    addFreq3dp(com1Standby, 523, 19);
    addFreq2dp(nav1Freq, 837, 19);
    addFreq2dp(nav1Standby, 1153, 19);

    // Add panel 2 frequencies
    addFreq3dp(com2Freq, 237, 148);
    addFreq3dp(com2Standby, 523, 148);
    addFreq2dp(nav2Freq, 837, 148);
    addFreq2dp(nav2Standby, 1153, 148);

    // Add panel 3 frequencies
    addNum4(simVars->adfFreq, 273, 278);
    addNum4(simVars->adfStandby, 586, 278);

    // Add squawk
    addSquawk(simVars->transponderCode, 968, 278);

    // Add selected switch
    switch (switchSel) {
    case 0:
        al_draw_scaled_bitmap(bitmaps[6], 0, 0, 80, 34, 460 * scaleFactor, 104 * scaleFactor, 80 * scaleFactor, 34 * scaleFactor, 0);
        break;
    case 1:
        al_draw_scaled_bitmap(bitmaps[6], 0, 0, 80, 34, 1064 * scaleFactor, 104 * scaleFactor, 80 * scaleFactor, 34 * scaleFactor, 0);
        break;
    case 2:
        al_draw_scaled_bitmap(bitmaps[6], 0, 0, 80, 34, 460 * scaleFactor, 233 * scaleFactor, 80 * scaleFactor, 34 * scaleFactor, 0);
        break;
    case 3:
        al_draw_scaled_bitmap(bitmaps[6], 0, 0, 80, 34, 1064 * scaleFactor, 233 * scaleFactor, 80 * scaleFactor, 34 * scaleFactor, 0);
        break;
    case 4:
        al_draw_scaled_bitmap(bitmaps[6], 0, 0, 80, 34, 460 * scaleFactor, 363 * scaleFactor, 80 * scaleFactor, 34 * scaleFactor, 0);
        break;
    }

    // Add transponder state
    int statePos = 80 * transponderState;
    if (switchSel == 5) {
        // Add transponder state selected
        al_draw_scaled_bitmap(bitmaps[7], statePos, 0, 80, 34, 1064 * scaleFactor, 363 * scaleFactor, 80 * scaleFactor, 34 * scaleFactor, 0);
    }
    else {
        // Add transponder state
        al_draw_scaled_bitmap(bitmaps[8], statePos, 0, 80, 34, 1064 * scaleFactor, 363 * scaleFactor, 80 * scaleFactor, 34 * scaleFactor, 0);
    }
}

/// <summary>
/// Draw the Autopilot at the stored position
/// </summary>
void nav::renderAutopilot()
{
    // Add main autopilot
    al_draw_bitmap(bitmaps[3], 0, 0, 0);

    // Add autopilot switch selected
    int selPos = 80 * (switchSel - 6);
    int destPos = 443 + 160 * (switchSel - 6);
    al_draw_scaled_bitmap(bitmaps[9], selPos, 0, 80, 34, destPos * scaleFactor, 340 * scaleFactor, 80 * scaleFactor, 34 * scaleFactor, 0);

    int destSizeX = 128 * scaleFactor;
    int destSizeY = 50 * scaleFactor;

    // Add autopilot set values
    if (autopilotSpd == SpdHold) {
        if (showMach) {
            addNum2dp(machX100, 421, 82);
        }
        else {
            addNum4(airspeed, 403, 82, false);
        }
    }
    addNum3(heading, 816, 82);
    addNum5(altitude, 1188, 82, false);
    
    // Add hdg display
    switch (autopilotHdg) {
    case HdgSet:
        al_draw_scaled_bitmap(bitmaps[10], 0, 0, 128, 50, 385 * scaleFactor, 252 * scaleFactor, destSizeX, destSizeY, 0);
        break;
    case LevelFlight:
        al_draw_scaled_bitmap(bitmaps[10], 128, 0, 128, 50, 385 * scaleFactor, 252 * scaleFactor, destSizeX, destSizeY, 0);
        break;
    }

    // Add ap display
    if (simVars->autopilotEngaged) {
        al_draw_scaled_bitmap(bitmaps[10], 256, 0, 128, 50, 530 * scaleFactor, 252 * scaleFactor, destSizeX, destSizeY, 0);
    }

    // Add alt display
    switch (autopilotAlt) {
    case AltHold:
        al_draw_scaled_bitmap(bitmaps[10], 384, 0, 128, 50, 680 * scaleFactor, 252 * scaleFactor, destSizeX, destSizeY, 0);
        break;
    case PitchHold:
        al_draw_scaled_bitmap(bitmaps[10], 512, 0, 128, 50, 680 * scaleFactor, 252 * scaleFactor, destSizeX, destSizeY, 0);
        break;
    case VerticalSpeedHold:
    {
        al_draw_scaled_bitmap(bitmaps[10], 640, 0, 128, 50, 680 * scaleFactor, 252 * scaleFactor, destSizeX, destSizeY, 0);
        addVerticalSpeed(836, 252);
        // Add white alts display
        al_draw_scaled_bitmap(bitmaps[10], 896, 0, 128, 50, 1115 * scaleFactor, 252 * scaleFactor, destSizeX, destSizeY, 0);
        break;
    }
    case AltChange:
        // ALT
        al_draw_scaled_bitmap(bitmaps[10], 384, 0, 128, 50, 680 * scaleFactor, 252 * scaleFactor, destSizeX, destSizeY, 0);
        // + S = ALTS
        al_draw_scaled_bitmap(bitmaps[10], 692, 0, 32, 50, 788 * scaleFactor, 252 * scaleFactor, 32 * scaleFactor, destSizeY, 0);
        // Add white alt display
        al_draw_scaled_bitmap(bitmaps[10], 896, 0, 94, 50, 1115 * scaleFactor, 252 * scaleFactor, 94 * scaleFactor, destSizeY, 0);
        break;
    }
}

/// <summary>
/// Displays a 3 digit number
/// </summary>
void nav::addNum3(int val, int x, int y)
{
    int digit1 = (val % 1000) / 100;
    int digit2 = (val % 100) / 10;
    int digit3 = val % 10;

    int yPos = y * scaleFactor;
    int width = 38 * scaleFactor;
    int height = 80 * scaleFactor;

    al_draw_scaled_bitmap(bitmaps[4], 38 * digit1, 0, 38, 80, x * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit2, 0, 38, 80, (x + 38) * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit3, 0, 38, 80, (x + 76) * scaleFactor, yPos, width, height, 0);
}

/// <summary>
/// Displays a 4 digit number
/// </summary>
void nav::addNum4(int val, int x, int y, bool leading)
{
    if (!leading && val == 0) {
        return;
    }

    int digit1 = (val % 10000) / 1000;
    int digit2 = (val % 1000) / 100;
    int digit3 = (val % 100) / 10;
    int digit4 = val % 10;

    int yPos = y * scaleFactor;
    int width = 38 * scaleFactor;
    int height = 80 * scaleFactor;

    if (leading || digit1 != 0) {
        al_draw_scaled_bitmap(bitmaps[4], 38 * digit1, 0, 38, 80, x * scaleFactor, yPos, width, height, 0);
    }
    x += 38;

    if (leading || digit1 != 0 || digit2 != 0) {
        al_draw_scaled_bitmap(bitmaps[4], 38 * digit2, 0, 38, 80, x * scaleFactor, yPos, width, height, 0);
    }
    x += 38;

    al_draw_scaled_bitmap(bitmaps[4], 38 * digit3, 0, 38, 80, x * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit4, 0, 38, 80, (x + 38) * scaleFactor, yPos, width, height, 0);
}

/// <summary>
/// Displays a 5 digit number
/// </summary>
void nav::addNum5(int val, int x, int y, bool leading)
{
    if (!leading && val == 0) {
        return;
    }

    int digit1 = (val % 100000) / 10000;
    int digit2 = (val % 10000) / 1000;
    int digit3 = (val % 1000) / 100;
    int digit4 = (val % 100) / 10;
    int digit5 = val % 10;

    int yPos = y * scaleFactor;
    int width = 38 * scaleFactor;
    int height = 80 * scaleFactor;

    if (leading || digit1 != 0) {
        al_draw_scaled_bitmap(bitmaps[4], 38 * digit1, 0, 38, 80, x * scaleFactor, yPos, width, height, 0);
    }
    x += 38;

    if (leading || digit1 != 0 || digit2 != 0) {
        al_draw_scaled_bitmap(bitmaps[4], 38 * digit2, 0, 38, 80, x * scaleFactor, yPos, width, height, 0);
    }
    x += 38;

    al_draw_scaled_bitmap(bitmaps[4], 38 * digit3, 0, 38, 80, x * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit4, 0, 38, 80, (x + 38) * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit5, 0, 38, 80, (x + 76) * scaleFactor, yPos, width, height, 0);
}

/// <summary>
/// Displays a value (number * 100) to 2 d.p.
/// </summary>
void nav::addNum2dp(int val, int x, int y)
{
    int digit1 = (val % 1000) / 100;
    int digit2 = (val % 100) / 10;
    int digit3 = val % 10;

    int yPos = y * scaleFactor;
    int width = 38 * scaleFactor;
    int height = 80 * scaleFactor;

    al_draw_scaled_bitmap(bitmaps[4], 38 * digit1, 0, 38, 80, x * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[5], 0, 0, 20, 80, (x + 38) * scaleFactor, yPos, 20 * scaleFactor, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit2, 0, 38, 80, (x + 58) * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit3, 0, 38, 80, (x + 96) * scaleFactor, yPos, width, height, 0);
}

/// <summary>
/// Displays the specified frequency to 2 d.p.
/// </summary>
void nav::addFreq2dp(int freq, int x, int y)
{
    int digit1 = freq / 10000;
    int digit2 = (freq % 10000) / 1000;
    int digit3 = (freq % 1000) / 100;
    int digit4 = (freq % 100) / 10;
    int digit5 = freq % 10;

    int yPos = y * scaleFactor;
    int width = 38 * scaleFactor;
    int height = 80 * scaleFactor;

    al_draw_scaled_bitmap(bitmaps[4], 38 * digit1, 0, 38, 80, x * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit2, 0, 38, 80, (x + 38) * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit3, 0, 38, 80, (x + 76) * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[5], 0, 0, 20, 80, (x + 114) * scaleFactor, yPos, 20 * scaleFactor, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit4, 0, 38, 80, (x + 134) * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit5, 0, 38, 80, (x + 172) * scaleFactor, yPos, width, height, 0);
}

/// <summary>
/// Displays the specified frequency to 3 d.p.
/// </summary>
void nav::addFreq3dp(int freq, int x, int y)
{
    int digit1 = freq / 100000;
    int digit2 = (freq % 100000) / 10000;
    int digit3 = (freq % 10000) / 1000;
    int digit4 = (freq % 1000) / 100;
    int digit5 = (freq % 100) / 10;
    int digit6 = freq % 10;

    int yPos = y * scaleFactor;
    int width = 38 * scaleFactor;
    int height = 80 * scaleFactor;

    al_draw_scaled_bitmap(bitmaps[4], 38 * digit1, 0, 38, 80, x * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit2, 0, 38, 80, (x + 38) * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit3, 0, 38, 80, (x + 76) * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[5], 0, 0, 20, 80, (x + 114) * scaleFactor, yPos, 20 * scaleFactor, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit4, 0, 38, 80, (x + 134) * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit5, 0, 38, 80, (x + 172) * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit6, 0, 38, 80, (x + 210) * scaleFactor, yPos, width, height, 0);
}

/// <summary>
/// Displays the squawk code
/// </summary>
void nav::addSquawk(int code, int x, int y)
{
    // Transponder code is in BCO16
    int digit1 = code / 4096;
    code -= digit1 * 4096;
    int digit2 = code / 256;
    code -= digit2 * 256;
    int digit3 = code / 16;
    int digit4 = code - digit3 * 16;

    int yPos = y * scaleFactor;
    int width = 38 * scaleFactor;
    int height = 80 * scaleFactor;

    al_draw_scaled_bitmap(bitmaps[4], 38 * digit1, 0, 38, 80, x * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit2, 0, 38, 80, (x + 76) * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit3, 0, 38, 80, (x + 152) * scaleFactor, yPos, width, height, 0);
    al_draw_scaled_bitmap(bitmaps[4], 38 * digit4, 0, 38, 80, (x + 228) * scaleFactor, yPos, width, height, 0);
}

void nav::addVerticalSpeed(int x, int y)
{
    int yPos = y * scaleFactor;
    int height = 50 * scaleFactor;

    if (simVars->autopilotVerticalSpeed == 0) {
        // Add 0fpm
        x += 87;
        al_draw_scaled_bitmap(bitmaps[12], 32, 0, 130, 50, x * scaleFactor, yPos, 162 * scaleFactor, height, 0);
        return;
    }

    int val = abs(simVars->autopilotVerticalSpeed);
    int digit1 = (val % 10000) / 1000;
    int digit2 = (val % 1000) / 100;

    if (digit1 == 0) {
        x += 32;
    }

    if (simVars->autopilotVerticalSpeed < 0) {
        // Add minus
        al_draw_scaled_bitmap(bitmaps[13], 0, 0, 23, 50, x * scaleFactor, yPos, 23 * scaleFactor, height, 0);
    }
    x += 23;

    if (digit1 != 0) {
        al_draw_scaled_bitmap(bitmaps[11], 32 * digit1, 0, 32, 50, x * scaleFactor, yPos, 32 * scaleFactor, height, 0);
        x += 32;
    }

    al_draw_scaled_bitmap(bitmaps[11], 32 * digit2, 0, 32, 50, x * scaleFactor, yPos, 32 * scaleFactor, height, 0);
    x += 32;

    // Add 00fpm
    al_draw_scaled_bitmap(bitmaps[12], 0, 0, 162, 50, x * scaleFactor, yPos, 162 * scaleFactor, height, 0);
}

/// <summary>
/// Fetch flightsim vars and then update all internal variables
/// that affect this instrument.
/// </summary>
void nav::update()
{
    // Check for position or size change
    long *settings = globals.simVars->readSettings(name, xPos, yPos, size);

    xPos = settings[0];
    yPos = settings[1];

    if (size != settings[2]) {
        size = settings[2];
        resize();
    }

#ifndef _WIN32
    // Only have hardware knobs on Raspberry Pi
    if (globals.hardwareKnobs) {
        updateKnobs();
    }
#endif

    // Calculate values - 3 d.p. for comms, 2 d.p. for nav, 0 d.p. for adf
    com1Freq = (simVars->com1Freq + 0.0000001) * 1000.0;
    com1Standby = (simVars->com1Standby + 0.0000001) * 1000.0;
    nav1Freq = (simVars->nav1Freq + 0.0000001) * 100.0;
    nav1Standby = (simVars->nav1Standby + 0.0000001) * 100.0;
    com2Freq = (simVars->com2Freq + 0.0000001) * 1000.0;
    com2Standby = (simVars->com2Standby + 0.0000001) * 1000.0;
    nav2Freq = (simVars->nav2Freq + 0.0000001) * 100.0;
    nav2Standby = (simVars->nav2Standby + 0.0000001) * 100.0;

    airspeed = simVars->autopilotAirspeed + 0.5;
    machX100 = simVars->autopilotMach * 100 + 0.5;
    heading = simVars->autopilotHeading + 0.5;
    altitude = simVars->autopilotAltitude + 0.5;

    if (simVars->autopilotAirspeedHold == 1) {
        autopilotSpd = SpdHold;
    }
    else {
        autopilotSpd = NoSpd;
    }

    if (simVars->autopilotHeadingLock == 1) {
        autopilotHdg = HdgSet;
    }
    else if (simVars->autopilotLevel == 1) {
        autopilotHdg = LevelFlight;
    }
    else {
        autopilotHdg = NoHdg;
    }

    if (simVars->autopilotAltLock == 1) {
        if (autopilotAlt == AltChange) {
            // Revert to alt hold when within range of target altitude
            int diff = abs(simVars->altAltitude - simVars->autopilotAltitude);
            if (diff < 210) {
                autopilotAlt = AltHold;
            }
        }
        else {
            autopilotAlt = AltHold;
        }
    }
    else if (simVars->autopilotVerticalHold == 1) {
        autopilotAlt = VerticalSpeedHold;
    }
    else if (simVars->autopilotPitchHold == 1) {
        autopilotAlt = PitchHold;
    }
    else {
        autopilotAlt = NoAlt;
    }
}

/// <summary>
/// Add FlightSim variables for this instrument (used for simulation mode)
/// </summary>
void nav::addVars()
{
    globals.simVars->addVar(name, "Com Active Frequency:1", false, 0.005, 100);
    globals.simVars->addVar(name, "Com Standby Frequency:1", false, 0.005, 100);
    globals.simVars->addVar(name, "Nav Active Frequency:1", false, 0.05, 100);
    globals.simVars->addVar(name, "Nav Standby Frequency:1", false, 0.05, 100);
    globals.simVars->addVar(name, "Com Active Frequency:2", false, 0.005, 100);
    globals.simVars->addVar(name, "Com Standby Frequency:2", false, 0.005, 100);
    globals.simVars->addVar(name, "Nav Active Frequency:2", false, 0.05, 100);
    globals.simVars->addVar(name, "Nav Standby Frequency:2", false, 0.05, 100);
    globals.simVars->addVar(name, "Adf Active Frequency:1", false, 1, 100);
    globals.simVars->addVar(name, "Adf Standby Frequency:1", false, 1, 100);
    globals.simVars->addVar(name, "Transponder Code:1", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Available", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Master", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Heading Lock Dir", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Heading Lock", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Wing Leveler", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Altitude Lock Var", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Altitude Lock", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Pitch Hold", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Vertical Hold Var", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Vertical Hold", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Airspeed Hold Var", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Mach Hold Var", false, 1, 0);
    globals.simVars->addVar(name, "Autopilot Airspeed Hold", false, 1, 0);
}

#ifndef _WIN32

void nav::addKnobs()
{
    // BCM GPIO 8 and 7
    selKnob = globals.hardwareKnobs->add(8, 7, -1, -1, 0);

    // BCM GPIO 12
    selPush = globals.hardwareKnobs->add(12, 0, -1, -1, 0);

    // BCM GPIO 20 and 21
    adjustKnob = globals.hardwareKnobs->add(20, 21, -1, -1, 0);

    // BCM GPIO 16
    adjustPush = globals.hardwareKnobs->add(16, 0, -1, -1, 0);
}

void nav::updateKnobs()
{
    // Read knob for switch selection
    int val = globals.hardwareKnobs->read(selKnob);
    if (val != INT_MIN) {
        // Convert knob value to selection (adjust for desired sensitivity)
        int maxSwitch;
        if (simVars->autopilotAvailable) {
            maxSwitch = 10;
        }
        else {
            maxSwitch = 5;
        }

        int diff = (prevSelVal - val) / 2;
        if (diff > 0) {
            if (switchSel < maxSwitch) {
                switchSel++;
            }
            else {
                switchSel = 0;
            }
            prevSelVal = val;
            adjustSetSel = 0;
        }
        else if (diff < 0) {
            if (switchSel > 0) {
                switchSel--;
            }
            else {
                switchSel = maxSwitch;
            }
            prevSelVal = val;
            adjustSetSel = 0;
        }
    }

    // Read switch push
    val = globals.hardwareKnobs->read(selPush);
    if (val != INT_MIN) {
        // If previous state was unpressed then must have been pressed
        if (prevSelPush % 2 == 1) {
            if (switchSel < 6) {
                navSwitchPressed();
            }
            else {
                autopilotSwitchPressed();
            }
        }
        prevSelPush = val;
        adjustSetSel = 0;
    }

    // Read knob for digits set
    val = globals.hardwareKnobs->read(adjustKnob);
    if (val != INT_MIN) {
        int diff = (val - prevAdjustVal) / 2;
        int adjust = 0;
        if (diff > 0) {
            adjust = 1;
        }
        else if (diff < 0) {
            adjust = -1;
        }

        if (adjust != 0) {
            if (switchSel < 6) {
                navAdjustDigits(adjust);
            }
            else {
                autopilotAdjustDigits(adjust);
            }
            prevAdjustVal = val;
        }
        time(&lastAdjust);
    }
    else if (lastAdjust != 0) {
        // Reset digit set selection if more than 5 seconds since last adjustment
        time(&now);
        if (now - lastAdjust > 5) {
            adjustSetSel = 0;
            lastAdjust = 0;
        }
    }

    // Read digits set push
    val = globals.hardwareKnobs->read(adjustPush);
    if (val != INT_MIN) {
        // If previous state was unpressed then must have been pressed
        if (prevAdjustPush % 2 == 1) {
            int digitSets;
            if (switchSel == 0 || switchSel == 2 || switchSel == 4) {
                digitSets = 3;
            }
            else if (switchSel == 5) {
                digitSets = 4;
            }
            else {
                digitSets = 2;
            }

            adjustSetSel++;
            if (adjustSetSel >= digitSets) {
                adjustSetSel = 0;
            }
        }
        prevAdjustPush = val;
    }
}

void nav::navSwitchPressed()
{
    // Swap standby and primary values
    switch (switchSel) {
    case 0:
    {
        globals.simVars->write(KEY_COM_RADIO_SWAP);
        break;
    }
    case 1:
    {
        globals.simVars->write(KEY_NAV1_RADIO_SWAP);
        break;
    }
    case 2:
    {
        globals.simVars->write(KEY_COM2_RADIO_SWAP);
        break;
    }
    case 3:
    {
        globals.simVars->write(KEY_NAV2_RADIO_SWAP);
        break;
    }
    case 4:
    {
        int newFreq = simVars->adfStandby;
        globals.simVars->write(KEY_ADF_COMPLETE_SET, simVars->adfFreq);
        globals.simVars->write(KEY_ADF1_PRIMARY_SET, newFreq);
        break;
    }
    case 5:
    {
        if (transponderState == 3) {
            transponderState = 0;
        }
        else {
            transponderState++;
        }
        break;
    }
    }
}

void nav::autopilotSwitchPressed()
{
    switch (switchSel) {
    case 6:
    {
        globals.simVars->write(KEY_AP_MASTER);
        break;
    }
    case 7:
    {
        if (autopilotSpd == SpdHold) {
            // Switch between knots and mach display.
            // Sets currently displayed value before switching to
            // set correctly converted value for current altitude.
            if (showMach) {
                // For some weird reason you have to set mach * 100 !
                globals.simVars->write(KEY_AP_MACH_VAR_SET, simVars->autopilotMach * 100);
                showMach = false;
            }
            else {
                globals.simVars->write(KEY_AP_SPD_VAR_SET, simVars->autopilotAirspeed);
                showMach = true;
            }
        }
        else {
            // Switch to Airspeed hold.
            // Set autopilot speed to within 10 knots of current speed
            int holdSpeed = simVars->asiAirspeed;
            int tens = holdSpeed % 10;
            if (tens < 5) {
                holdSpeed -= tens;
            }
            else {
                holdSpeed += 10 - tens;
            }
            globals.simVars->write(KEY_AP_SPD_VAR_SET, holdSpeed);
            globals.simVars->write(KEY_AP_AIRSPEED_ON);
            showMach = false;
        }
        break;
    }
    case 8:
    {
        if (autopilotHdg == HdgSet) {
            autopilotHdg = LevelFlight;
            globals.simVars->write(KEY_AP_HDG_HOLD_OFF);
        }
        else {
            autopilotHdg = HdgSet;
            globals.simVars->write(KEY_AP_HDG_HOLD_ON);
        }
        break;
    }
    case 9:
    {
        if (autopilotAlt == AltHold) {
            autopilotAlt = PitchHold;
            globals.simVars->write(KEY_AP_ALT_HOLD_OFF);
        }
        else {
            autopilotAlt = AltHold;
            // Set autopilot altitude to within 100ft of current altitude
            int holdAlt = simVars->altAltitude;
            int hundreds = holdAlt % 100;
            if (hundreds < 30) {
                holdAlt -= hundreds;
            }
            else {
                holdAlt += 100 - hundreds;
            }
            globals.simVars->write(KEY_AP_ALT_VAR_SET_ENGLISH, holdAlt);
            globals.simVars->write(KEY_AP_ALT_HOLD_ON);
        }
        break;
    }
    case 10:
    {
        // Vertical speed hold not working so set target altitude instead
        autopilotAlt = AltChange;
        globals.simVars->write(KEY_AP_ALT_VAR_SET_ENGLISH, simVars->autopilotAltitude);
        globals.simVars->write(KEY_AP_ALT_HOLD_ON);
        break;
    }
    }
}

void nav::navAdjustDigits(int adjust)
{
    switch (switchSel) {
    case 0:
    {
        double newVal = adjustCom(simVars->com1Standby, adjust);
        globals.simVars->write(KEY_COM_STBY_RADIO_SET, newVal);
        break;
    }
    case 1:
    {
        double newVal = adjustNav(simVars->nav1Standby, adjust);
        globals.simVars->write(KEY_NAV1_STBY_SET, newVal);
        break;
    }
    case 2:
    {
        double newVal = adjustCom(simVars->com2Standby, adjust);
        globals.simVars->write(KEY_COM2_STBY_RADIO_SET, newVal);
        break;
    }
    case 3:
    {
        double newVal = adjustNav(simVars->nav2Standby, adjust);
        globals.simVars->write(KEY_NAV2_STBY_SET, newVal);
        break;
    }
    case 4:
    {
        int newVal = adjustAdf(simVars->adfStandby, adjust);
        globals.simVars->write(KEY_ADF_COMPLETE_SET, newVal);
        break;
    }
    case 5:
    {
        int newVal = adjustSquawk(simVars->transponderCode, adjust);
        globals.simVars->write(KEY_XPNDR_SET, newVal);
        break;
    }
    }
}

void nav::autopilotAdjustDigits(int adjust)
{
    switch (switchSel) {
    case 7:
    {
        if (autopilotSpd == SpdHold) {
            if (showMach) {
                double newVal = adjustMach(simVars->autopilotMach, adjust);
                globals.simVars->write(KEY_AP_MACH_VAR_SET, newVal);
            }
            else {
                double newVal = adjustSpeed(simVars->autopilotAirspeed, adjust);
                globals.simVars->write(KEY_AP_SPD_VAR_SET, newVal);
            }
        }
        break;
    }
    case 8:
    {
        double newVal = adjustHeading(simVars->autopilotHeading, adjust);
        globals.simVars->write(KEY_HEADING_BUG_SET, newVal);
        break;
    }
    case 9:
    {
        double newVal = adjustAltitude(simVars->autopilotAltitude, adjust);
        globals.simVars->write(KEY_AP_ALT_VAR_SET_ENGLISH, newVal);
        break;
    }
    case 10:
    {
        //double newVal = adjustVerticalSpeed(simVars->autopilotVerticalSpeed, adjust);
        //globals.simVars->write(KEY_AP_VS_VAR_SET_ENGLISH, newVal);

        // Adjust altitude instead of vertical speed for now
        double newVal = adjustAltitude(simVars->autopilotAltitude, adjust);
        globals.simVars->write(KEY_AP_ALT_VAR_SET_ENGLISH, newVal);
        break;
    }
    }
}

double nav::adjustCom(double val, int adjust)
{
    int whole = val;
    val -= whole;
    int thousandths = (val + 0.0001) * 1000;
    int frac1 = thousandths / 100;
    int frac2 = thousandths % 100;

    if (adjustSetSel == 0) {
        // Adjust whole - Range 118 to 136
        whole += adjust;
        if (whole > 136) {
            whole -= 19;
        }
        else if (whole < 118) {
            whole += 19;
        }
    }
    else if (adjustSetSel == 1) {
        // Adjust 10ths
        frac1 += adjust;
        if (frac1 > 9) {
            frac1 -= 10;
        }
        else if (frac1 < 0) {
            frac1 += 10;
        }
    }
    else {
        // Adjust 100ths and 1000ths
        frac2 += adjust * 5;

        // Skip .020, .045, .070 and .095
        if (frac2 == 20 || frac2 == 45 || frac2 == 70 || frac2 == 95) {
            frac2 += adjust * 5;
        }

        if (frac2 >= 100) {
            frac2 -= 100;
        }
        else if (frac2 < 0) {
            frac2 += 100;
        }
    }

    return whole + frac1 * 0.1 + frac2 * 0.001;
}

double nav::adjustNav(double val, int adjust)
{
    int whole = val;
    val -= whole;
    int frac = (val + 0.001) * 100;

    if (adjustSetSel == 0) {
        // Adjust whole - Range 108 to 117
        whole += adjust;
        if (whole > 117) {
            whole -= 10;
        }
        else if (whole < 108) {
            whole += 10;
        }
    }
    else {
        // Adjust fraction
        frac += adjust * 5;

        if (frac >= 100) {
            frac -= 100;
        }
        else if (frac < 0) {
            frac += 100;
        }
    }

    return whole + frac * 0.01;
}

int nav::adjustAdf(int val, int adjust)
{
    if (adjustSetSel == 0) {
        val += adjust * 100;
        if (val >= 1800) {
            val -= 1700;
        }
        else if (val < 100) {
            val += 1700;
        }
    }
    else if (adjustSetSel == 1) {
        // Adjust 3rd digit
        int digit = adjustDigit(((int)val % 100) / 10, adjust);
        val = (int)(val / 100) * 100 + digit * 10 + (val % 10);
    }
    else {
        // Adjust 4th digit
        int digit = adjustDigit((int)val % 10, adjust);
        val = (int)(val / 10) * 10 + digit;
    }

    return val;
}

int nav::adjustSquawk(int val, int adjust)
{
    // Transponder code is in BCO16
    int digit1 = val / 4096;
    val -= digit1 * 4096;
    int digit2 = val / 256;
    val -= digit2 * 256;
    int digit3 = val / 16;
    int digit4 = val - digit3 * 16;

    switch (adjustSetSel) {
    case 0:
        digit1 = adjustDigit(digit1, adjust, true);
        break;
    case 1:
        digit2 = adjustDigit(digit2, adjust, true);
        break;
    case 2:
        digit3 = adjustDigit(digit3, adjust, true);
        break;
    case 3:
        digit4 = adjustDigit(digit4, adjust, true);
        break;
    }

    return digit1 * 4096 + digit2 * 256 + digit3 * 16 + digit4;
}

int nav::adjustSpeed(int val, int adjust)
{
    if (adjustSetSel == 0) {
        // Adjust tens
        val += adjust * 10;
    }
    else {
        // Adjust units
        int digit = adjustDigit(val % 10, adjust);
        val = (int)(val / 10) * 10 + digit;
    }

    return val;
}

double nav::adjustMach(double val, int adjust)
{
    int whole = val;
    val -= whole;
    int frac = val * 100 + 0.5;

    // Default to adjusting fraction first on mach
    if (adjustSetSel == 0) {
        // Adjust fraction
        frac += adjust;

        if (frac >= 100) {
            frac -= 100;
        }
        else if (frac < 0) {
            frac += 100;
        }
    }
    else {
        // Adjust whole
        whole += adjust;
        if (whole > 2) {
            whole -= 3;
        }
        else if (whole < 0) {
            whole += 3;
        }
    }

    // For some weird reason you have to set mach * 100 !
    return whole * 100 + frac;
}

int nav::adjustHeading(int val, int adjust)
{
    if (adjustSetSel == 0) {
        // Adjust tens
        val += adjust * 10;

        if (val > 359) {
            val -= 360;
        }
        else if (val < 0) {
            val += 360;
        }
    }
    else {
        // Adjust units
        int digit = adjustDigit(val % 10, adjust);
        val = (int)(val / 10) * 10 + digit;
    }

    return val;
}

int nav::adjustAltitude(int val, int adjust)
{
    int prevVal = val;

    if (adjustSetSel == 0) {
        // Adjust thousands
        val += adjust * 1000;

        if (val < 0) {
            val += 1000;
        }
    }
    else {
        // Adjust hundreds
        int digit = adjustDigit(((int)val % 1000) / 100, adjust);
        val = (int)(val / 1000) * 1000 + digit * 100 + (val % 100);

        if (val < 0) {
            val += 100;
        }
    }

    if (autopilotAlt == AltChange) {
        // Check for cancel alt change
        int diff = abs(val - simVars->altAltitude);
        if (diff < 210 || (val < simVars->altAltitude && prevVal > simVars->altAltitude)
            || (val > simVars->altAltitude && prevVal < simVars->altAltitude)) {
            autopilotAlt = AltHold;
        }
    }

    return val;
}

int nav::adjustVerticalSpeed(int val, int adjust)
{
    // Allow vertical speed to go negative
    val += adjust * 100;

    return val;
}

int nav::adjustDigit(int val, int adjust, bool isSquawk)
{
    int maxDigit;
    if (isSquawk) {
        maxDigit = 7;
    }
    else {
        maxDigit = 9;
    }

    val += adjust;
    if (val > maxDigit) {
        val -= maxDigit + 1;
    }
    else if (val < 0) {
        val += maxDigit + 1;
    }

    return val;
}

#endif // !_WIN32
