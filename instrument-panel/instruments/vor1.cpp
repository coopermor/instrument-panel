#include <stdio.h>
#include <stdlib.h>
#include "vor1.h"
#include "simvars.h"
#include "knobs.h"

vor1::vor1(int xPos, int yPos, int size) : instrument(xPos, yPos, size)
{
    setName("VOR1");
    addVars();

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
void vor1::resize()
{
    destroyBitmaps();

    // Create bitmaps scaled to correct size (original size is 800)
    scaleFactor = size / 800.0f;

    // 0 = Original (loaded) bitmap
    ALLEGRO_BITMAP* orig = loadBitmap("vor1.png");
    addBitmap(orig);

    if (bitmaps[0] == NULL) {
        return;
    }

    // 1 = Destination bitmap (all other bitmaps get assembled to here)
    ALLEGRO_BITMAP* bmp = al_create_bitmap(size, size);
    addBitmap(bmp);

    // 2 = Back
    bmp = al_create_bitmap(size, size);
    al_set_target_bitmap(bmp);
    al_draw_scaled_bitmap(orig, 0, 0, 800, 800, 0, 0, size, size, 0);
    addBitmap(bmp);

    // 3 = Compass
    bmp = al_create_bitmap(800, 800);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 800, 0, 800, 800, 0, 0, 0);
    addBitmap(bmp);

    // 4 = Glide slope on
    bmp = al_create_bitmap(100 * scaleFactor, 50 * scaleFactor);
    al_set_target_bitmap(bmp);
    al_draw_scaled_bitmap(orig, 800, 800, 100, 50, 0, 0, 100 * scaleFactor, 50 * scaleFactor, 0);
    addBitmap(bmp);

    // 5 = From on
    bmp = al_create_bitmap(100 * scaleFactor, 50 * scaleFactor);
    al_set_target_bitmap(bmp);
    al_draw_scaled_bitmap(orig, 900, 800, 100, 50, 0, 0, 100 * scaleFactor, 50 * scaleFactor, 0);
    addBitmap(bmp);

    // 6 = To on
    bmp = al_create_bitmap(100 * scaleFactor, 50 * scaleFactor);
    al_set_target_bitmap(bmp);
    al_draw_scaled_bitmap(orig, 1000, 800, 100, 50, 0, 0, 100 * scaleFactor, 50 * scaleFactor, 0);
    addBitmap(bmp);

    // 7 = Locator needle
    bmp = al_create_bitmap(30, 800);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 1600, 0, 30, 800, 0, 0, 0);
    addBitmap(bmp);

    // 8 = Glide slope needle
    bmp = al_create_bitmap(800, 30);
    al_set_target_bitmap(bmp);
    al_draw_bitmap_region(orig, 0, 800, 800, 30, 0, 0, 0);
    addBitmap(bmp);

    // 9 = Top guide
    bmp = al_create_bitmap(70 * scaleFactor, 180 * scaleFactor);
    al_set_target_bitmap(bmp);
    al_draw_scaled_bitmap(orig, 1630, 0, 70, 180, 0, 0, 70 * scaleFactor, 180 * scaleFactor, 0);
    addBitmap(bmp);

    // 10 = Bottom guide
    bmp = al_create_bitmap(70 * scaleFactor, 180 * scaleFactor);
    al_set_target_bitmap(bmp);
    al_draw_scaled_bitmap(orig, 1630, 180, 70, 180, 0, 0, 70 * scaleFactor, 180 * scaleFactor, 0);
    addBitmap(bmp);

    al_set_target_backbuffer(globals.display);
}

/// <summary>
/// Draw the instrument at the stored position
/// </summary>
void vor1::render()
{
    if (bitmaps[0] == NULL) {
        return;
    }

    // Use normal blender
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    // Draw stuff into dest bitmap
    al_set_target_bitmap(bitmaps[1]);

    // Add back
    al_draw_bitmap(bitmaps[2], 0, 0, 0);

    // Add glide slope on
    if (glideSlopeOn == 1) {
        al_draw_bitmap(bitmaps[4], 490 * scaleFactor, 335 * scaleFactor, 0);
    }

    // Add to/from on
    if (toFromOn == 1) {
        al_draw_bitmap(bitmaps[6], 350 * scaleFactor, 549 * scaleFactor, 0);
    }
    else if (toFromOn == 2) {
        al_draw_bitmap(bitmaps[5], 350 * scaleFactor, 549 * scaleFactor, 0);
    }

    // Add locator needle
    al_draw_scaled_rotated_bitmap(bitmaps[7], 15, 140, 400 * scaleFactor, 140 * scaleFactor, scaleFactor, scaleFactor, locAngle * DegreesToRadians, 0);

    // Add glide slope needle
    al_draw_scaled_rotated_bitmap(bitmaps[8], 140, 15, 140 * scaleFactor, 400 * scaleFactor, scaleFactor, scaleFactor, slopeAngle * DegreesToRadians, 0);

    // Add compass
    al_draw_scaled_rotated_bitmap(bitmaps[3], 400, 400, 400 * scaleFactor, 400 * scaleFactor, scaleFactor, scaleFactor, compassAngle * DegreesToRadians, 0);

    // Add top guide
    al_draw_bitmap(bitmaps[9], 365 * scaleFactor, 0, 0);

    // Add bottom guide
    al_draw_bitmap(bitmaps[10], 365 * scaleFactor, 620 * scaleFactor, 0);

    // Position dest bitmap on screen
    al_set_target_backbuffer(globals.display);
    al_draw_bitmap(bitmaps[1], xPos, yPos, 0);

    if (!globals.active) {
        dimInstrument();
    }
}

/// <summary>
/// Fetch flightsim vars and then update all internal variables
/// that affect this instrument.
/// </summary>
void vor1::update()
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

    // Get latest FlightSim variables
    SimVars* simVars = &globals.simVars->simVars;

    // Calculate values
    compassAngle = -simVars->vor1Obs;
    locAngle = -simVars->vor1RadialError * 15.0;
    slopeAngle = 50;
    toFromOn = simVars->vor1ToFrom;
    glideSlopeOn = simVars->vor1GlideSlopeFlag;

    if (abs(locAngle) > 50) {
        if (locAngle > 0) locAngle = 50; else locAngle = -50;
    }

    slopeAngle = simVars->vor1GlideSlopeError * 25.0;
    if (abs(slopeAngle) > 50) {
        if (slopeAngle > 0) slopeAngle = 50; else slopeAngle = -50;
    }
}

/// <summary>
/// Add FlightSim variables for this instrument (used for simulation mode)
/// </summary>
void vor1::addVars()
{
    globals.simVars->addVar(name, "Nav Obs:1", false, 1, 0);
    globals.simVars->addVar(name, "Nav Radial Error:1", false, 1, 0);
    globals.simVars->addVar(name, "Nav Glide Slope Error:1", false, 1, 0);
    globals.simVars->addVar(name, "Nav ToFrom:1", false, 1, 0);
    globals.simVars->addVar(name, "Nav Gs Flag:1", false, 1, 0);
}

#ifndef _WIN32

void vor1::addKnobs()
{
    // BCM GPIO 11 and 5
    obsKnob = globals.hardwareKnobs->add(11, 5, -1, -1, 0);
}

void vor1::updateKnobs()
{
    // Read knob for new instrument calibration
    int val = globals.hardwareKnobs->read(obsKnob);

    if (val != INT_MIN) {
        // Change Obs by knob movement amount (adjust for desired sensitivity)
        int adjust = (int)((val - prevVal) / 2) * 5;
        if (adjust != 0) {
            double newVal = globals.simVars->simVars.vor1Obs + adjust;

            if (newVal < 0) {
                newVal += 360;
            }
            else if (newVal >= 360) {
                newVal -= 360;
            }
            globals.simVars->write(KEY_VOR1_SET, newVal);
            prevVal = val;
        }
    }
}

#endif // !_WIN32
