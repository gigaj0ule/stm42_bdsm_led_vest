// Headers
#include "sketch.hpp"
#include "sketch_only_statics.hpp"

// Libraries
#include <FastLED.h>


//#############################################################################
// Vest Layout
//
/*
           [L: 112-123]   [K: 92-111]
                       \ /
                  ----- O -----
             /          |          \
            /      [J: 87-91]       \
  [I: 75-86]            |            [F: 49-60]
           |            |            |
           O [G: 69-61] O [D: 36-43] O
          /             |             \
[H: 74-70]         [C: 33,34]         [E: 44-48]
                        |
                        O
                      /   \
            [B: 32-16]     [A: 15-0]
*/

//#############################################################################
// Functions
//
void paint_segment(
        uint16_t led_start,
        uint16_t led_end,
        uint8_t paint_brush[],
        uint16_t paint_brush_offset = 0
    ) {

    uint16_t plot_counter = 0;
    paint_brush_offset = paint_brush_offset * 3;

    if(led_start < led_end) {
        for(int16_t i = led_start; i < led_end; i++) {

            uint16_t three_times_startpoint = (3 * led_start);
            uint16_t three_step_counter = (3 * plot_counter);

            communicable.led_banks[0][three_times_startpoint + (three_step_counter + 0)] = paint_brush[paint_brush_offset + three_step_counter + 0];
            communicable.led_banks[0][three_times_startpoint + (three_step_counter + 1)] = paint_brush[paint_brush_offset + three_step_counter + 1];
            communicable.led_banks[0][three_times_startpoint + (three_step_counter + 2)] = paint_brush[paint_brush_offset + three_step_counter + 2];

            plot_counter ++;
        }
    }
    else {
        for(int16_t i = led_start; i > (led_end - 1); i--) {

            uint16_t three_times_endpoint = (3 * led_start);
            uint16_t three_step_counter = (3 * plot_counter);

            communicable.led_banks[0][three_times_endpoint - (three_step_counter - 0)] = paint_brush[paint_brush_offset + three_step_counter + 0];
            communicable.led_banks[0][three_times_endpoint - (three_step_counter - 1)] = paint_brush[paint_brush_offset + three_step_counter + 1];
            communicable.led_banks[0][three_times_endpoint - (three_step_counter - 2)] = paint_brush[paint_brush_offset + three_step_counter + 2];

            plot_counter ++;
        }
    }
}


//#############################################################################
// Declarations
//
using namespace NSFastLED;

NSFastLED::CRGB leds[NUMBER_OF_LEDS_TOTAL];
CLEDController *ledController[36];

#define DATA_PIN  PIN_SPI_MOSI // PA7
#define CLK_PIN   PIN_SPI_SCK // PA5

float led_intensity = 64.0f;


//#############################################################################
// Program
//
static void worker_thread(void* arg) {

    // Create a rainbow paintbrush!
    uint8_t rainbow_paintbrush[NUMBER_OF_BYTES_PER_LED_BANK] = {0};
    uint64_t radians_offset = 0;

    for(int j = 0; j < NUMBER_OF_LED_BANKS; j++) {

        // For every byte in a bank...
        for(int i = 0; i < NUMBER_OF_BYTES_PER_LED_BANK; i += 3) {

            radians_offset += 2;

            float radians = ((float) radians_offset) * ((PI * 2.0f) / NUMBER_OF_LEDS_PER_BANK);

            // Create colors
            float red = (led_intensity * ((sinf(radians)) + 1.0f) / 2.0f);
            float green = (led_intensity * ((sinf(radians + (2.0f * PI / 3.0f))) + 1.0f) / 2.0f);
            float blue = (led_intensity * ((sinf(radians - (2.0f * PI / 3.0f))) + 1.0f) / 2.0f);

            // Populate the index of LED array we're interested in
            rainbow_paintbrush[i + 0] = (uint8_t) green;
            rainbow_paintbrush[i + 1] = (uint8_t) red;
            rainbow_paintbrush[i + 2] = (uint8_t) blue;
        }
    }


    while(true) {

        // --------------------------------------------------------------
        // Cycle rainbow
        uint16_t rainbow_paintbrush_size = sizeof(rainbow_paintbrush) - 1;

        // Cycle the rainbow N+1
        for(int i = rainbow_paintbrush_size; i > 3; i -= 3) {
            rainbow_paintbrush[i - 0] = rainbow_paintbrush[i - 3];
            rainbow_paintbrush[i - 1] = rainbow_paintbrush[i - 4];
            rainbow_paintbrush[i - 2] = rainbow_paintbrush[i - 5];
        }

        // Pop last color
        uint8_t temp_g = rainbow_paintbrush[rainbow_paintbrush_size - 0];
        uint8_t temp_r = rainbow_paintbrush[rainbow_paintbrush_size - 1];
        uint8_t temp_b = rainbow_paintbrush[rainbow_paintbrush_size - 2];

        // Move first color to last
        rainbow_paintbrush[rainbow_paintbrush_size - 0] = rainbow_paintbrush[2];
        rainbow_paintbrush[rainbow_paintbrush_size - 1] = rainbow_paintbrush[1];
        rainbow_paintbrush[rainbow_paintbrush_size - 2] = rainbow_paintbrush[0];

        // Make first color the oldest (popped)
        rainbow_paintbrush[2] = temp_g;
        rainbow_paintbrush[1] = temp_r;
        rainbow_paintbrush[0] = temp_b;


        // --------------------------------------------------------------
        // Symmetric rainbow pattern, starting from the heart:

        // Section J, from heart to underboob
        paint_segment(90, 85, rainbow_paintbrush);

        // Section K, from heart over left shoulder
        paint_segment(91, 102, rainbow_paintbrush);

        // Section L, from heart over right shoulder
        paint_segment(102, 113, rainbow_paintbrush);

        // Section F, Crosses left boob
        paint_segment(59, 48, rainbow_paintbrush);

        // Section I, Crosses right boob
        paint_segment(85, 74, rainbow_paintbrush);

        // Section D, from belly to right
        paint_segment(34, 43, rainbow_paintbrush, 5);

        // Section G, from belly to left
        paint_segment(60, 69, rainbow_paintbrush, 5);

        // Section C, midbelly to navel
        paint_segment(33, 32, rainbow_paintbrush, 5);

        // Section E, from right to mid-back
        paint_segment(43, 48, rainbow_paintbrush, 14);

        // Section H, from left to mid-back
        paint_segment(69, 74, rainbow_paintbrush, 14);

        // Section B, from navel to right butt
        paint_segment(16, 32, rainbow_paintbrush, 7);

        // Section A, from navel to left butt
        paint_segment(15, 0, rainbow_paintbrush, 7);

        // --------------------------------------------------------------
        // Now illuminate every LED bank...
        for(int j = 0; j < NUMBER_OF_LED_BANKS; j++) {

            // For every byte in a bank...
            for(int i = 0; i < NUMBER_OF_BYTES_PER_LED_BANK; i += 3) {

                // Index of LED array we're interested in
                uint16_t led_index = (i / 3) + j * NUMBER_OF_LEDS_PER_BANK;

                // Turn the LED on, then pause
                leds[led_index]
                    = NSFastLED::CRGB(
                        *(communicable.led_banks[j] + i + 0),
                        *(communicable.led_banks[j] + i + 1),
                        *(communicable.led_banks[j] + i + 2)
                    );
            }
        }

        // Depending on the type of LED chosen, switch what is used here
        ledController[communicable.led_type]->showLeds();

        // --------------------------------------------------------------
        // Pass off this task to the scheduler
        os_delay(10000);
    }
}


//#############################################################################
// Pre-Scheduler Stuff
//
void setup() {

    // Init communication
    early_setup();

    // Init controllers
    ledController[0] = &FastLED.addLeds<LPD6803, DATA_PIN, CLK_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[1] = &FastLED.addLeds<LPD8806, DATA_PIN, CLK_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[2] = &FastLED.addLeds<WS2801, DATA_PIN, CLK_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[3] = &FastLED.addLeds<WS2803, DATA_PIN, CLK_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[4] = &FastLED.addLeds<SM16716, DATA_PIN, CLK_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[5] = &FastLED.addLeds<P9813, DATA_PIN, CLK_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[6] = &FastLED.addLeds<APA102, DATA_PIN, CLK_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[7] = &FastLED.addLeds<SK9822, DATA_PIN, CLK_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[8] = &FastLED.addLeds<DOTSTAR, DATA_PIN, CLK_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[9] = &FastLED.addLeds<SM16703, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);

    // 10
    ledController[10] = &FastLED.addLeds<TM1829, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[11] = &FastLED.addLeds<TM1812, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[12] = &FastLED.addLeds<TM1809, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[13] = &FastLED.addLeds<TM1804, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[14] = &FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[15] = &FastLED.addLeds<UCS1903, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[16] = &FastLED.addLeds<UCS1903B, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[17] = &FastLED.addLeds<UCS1904, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[18] = &FastLED.addLeds<UCS2903, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[19] = &FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);

    // 20
    ledController[20] = &FastLED.addLeds<WS2852, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[21] = &FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[22] = &FastLED.addLeds<GS1903, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[23] = &FastLED.addLeds<SK6812, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[24] = &FastLED.addLeds<APA106, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[25] = &FastLED.addLeds<PL9823, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[26] = &FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[27] = &FastLED.addLeds<WS2813, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[28] = &FastLED.addLeds<APA104, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[29] = &FastLED.addLeds<WS2811_400, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);

    // 30
    ledController[30] = &FastLED.addLeds<GE8822, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[31] = &FastLED.addLeds<GW6205, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[32] = &FastLED.addLeds<GW6205_400, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[33] = &FastLED.addLeds<LPD1886, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);
    ledController[34] = &FastLED.addLeds<LPD1886_8BIT, DATA_PIN, RGB>(leds, NUMBER_OF_LEDS_TOTAL);

    // Launch program!
    create_threads(worker_thread, 3, 3);
};


//#############################################################################
// Vestigal
//
void loop(){
    // Unused
};
