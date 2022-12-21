#include "plugin.hpp"

static const int BUFFER_SIZE = 1024;//256;

Vec polarToCart(Vec point){
    Vec new_point;
    new_point.x = point.y * sin(point.x);
    new_point.y = point.y * cos(point.x);
    return new_point;
}

struct PolarCV : Module {
    enum ParamId {
        SPEED_MULT_PARAM,
        PREV_EQ_PARAM,
        NEXT_EQ_PARAM,
        SPEED_DIAL_PARAM,
        A_DIAL_PARAM,
        B_DIAL_PARAM,
        CUR_EQ_PARAM,
        CUR_TIME_MULT_IDX_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        SPEED_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        R_OUT_OUTPUT,
        X_OUT_OUTPUT,
        Y_OUT_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        ENUMS(SPEED_MULT_LIGHT, 3),
        PREV_EQ_LIGHT,
        NEXT_EQ_LIGHT,
        LIGHTS_LEN
    };

    Vec current_cursor;
    float cur_theta = 0.1f;
    float theta_delta = 0.001f;
    int current_equation = 0;
    bool prev_next_eq_trig = false;
    bool prev_prev_eq_trig = false;
    float time_mults[3] = {0.5f, 1.f, 2.f};
    int color_lights[3][3] = {{255, 0, 255}, {0, 255, 255}, {255, 255, 0}};
    bool prev_time_mult = false;
    bool speed_input_connected = false;
    float speed = 0.f;
    float speed_range_slope = 1.0 * (0.02f - 0.0001f) / (0.f - 10.f);
    float theta_mod = M_PI * 128;
    
    struct Point {
        float minX[16] = {};
        float maxX[16] = {};
        float minY[16] = {};
        float maxY[16] = {};

        Point() {
            for (int c = 0; c < 16; c++) {
                minX[c] = INFINITY;
                maxX[c] = -INFINITY;
                minY[c] = INFINITY;
                maxY[c] = -INFINITY;
            }
        }
    };
    
    
	PolarCV() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configSwitch(SPEED_MULT_PARAM, 0.f, 1.f, 0.f, "");
        configSwitch(PREV_EQ_PARAM, 0.f, 1.f, 0.f, "");
        configSwitch(NEXT_EQ_PARAM, 0.f, 1.f, 0.f, "");
        configInput(SPEED_INPUT, "CV for Speed");
        configParam(SPEED_DIAL_PARAM,  0.0001f, 0.02f, 0.01f, "Speed", " screen/PI", 0.f, 0.0001f);
        configParam(A_DIAL_PARAM, 0, 20, 1, "A", " V", 0.f, 0.5f);
        configParam(B_DIAL_PARAM, 0, 20, 1, "B", " V", 0.f, 0.5f);
        configParam(CUR_EQ_PARAM, 0, 3, 1);
        configParam(CUR_TIME_MULT_IDX_PARAM, 0, 2, 1);
        getParamQuantity(A_DIAL_PARAM)->snapEnabled = true;
        getParamQuantity(B_DIAL_PARAM)->snapEnabled = true;
        configOutput(R_OUT_OUTPUT, "R");
        configOutput(X_OUT_OUTPUT, "X");
        configOutput(Y_OUT_OUTPUT, "Y");
	}

	void process(const ProcessArgs& args) override {
        int max_eqs = 4;
        
        bool double_time = params[SPEED_MULT_PARAM].getValue();
        if (prev_time_mult != double_time){
            params[CUR_TIME_MULT_IDX_PARAM].setValue((int(params[CUR_TIME_MULT_IDX_PARAM].getValue()) + 1) % 3);
        }
        prev_time_mult = double_time;
        
        speed_input_connected = inputs[SPEED_INPUT].isConnected();
        if (speed_input_connected){
            float speed_inp = inputs[SPEED_INPUT].getVoltage();
            speed_inp = clamp(speed_inp, 0.f, 10.f);
            speed = 0.0001f + speed_range_slope * (speed_inp - 0.f);
        }
        else{
            speed = params[SPEED_DIAL_PARAM].getValue();
        }

        bool next_trig = !params[NEXT_EQ_PARAM].getValue();
        if (next_trig != prev_next_eq_trig){
            params[CUR_EQ_PARAM].setValue((int(params[CUR_EQ_PARAM].getValue() + 1)) % max_eqs);
        }
        prev_next_eq_trig = next_trig;
        
        bool prev_trig = !params[PREV_EQ_PARAM].getValue();
        if (prev_trig != prev_prev_eq_trig){
            params[CUR_EQ_PARAM].setValue(params[CUR_EQ_PARAM].getValue() - 1);
            if (params[CUR_EQ_PARAM].getValue() < 0){
                params[CUR_EQ_PARAM].setValue(max_eqs - 1);
            }
        }
        prev_prev_eq_trig = prev_trig;

        //current Carts
        Vec carts = polarToCart(current_cursor);
        
        outputs[R_OUT_OUTPUT].setVoltage(current_cursor.y*10.0f);
        outputs[X_OUT_OUTPUT].setVoltage(carts.x*10.0f);
        outputs[Y_OUT_OUTPUT].setVoltage(carts.y*10.0f);
        
        int cur_mlt_idx = int(params[CUR_TIME_MULT_IDX_PARAM].getValue());
        lights[SPEED_MULT_LIGHT + 0].setBrightness(color_lights[cur_mlt_idx][0]);
        lights[SPEED_MULT_LIGHT + 1].setBrightness(color_lights[cur_mlt_idx][1]);
        lights[SPEED_MULT_LIGHT + 2].setBrightness(color_lights[cur_mlt_idx][2]);
        
        //lights[NEXT_EQ_LIGHT].setBrightness(params[NEXT_EQ_PARAM].getValue());
        lights[PREV_EQ_LIGHT].setBrightness(0.5f);
        lights[NEXT_EQ_LIGHT].setBrightness(0.5f);
        
        updateTheta();
	}
    
    void updateTheta() {
        
        float cur_mlt = time_mults[int(params[CUR_TIME_MULT_IDX_PARAM].getValue())];
        
        theta_delta = speed * M_PI / 1000.0f * (cur_mlt * cur_mlt) + 0.000005f;
        
        cur_theta = cur_theta + theta_delta;
        if (cur_theta > theta_mod){
            cur_theta = cur_theta - theta_mod;
        }
        
        float A = params[A_DIAL_PARAM].getValue() / 2.0f;
        float B = params[B_DIAL_PARAM].getValue() / 2.0f;
        
        float r;
        switch (int(params[CUR_EQ_PARAM].getValue())) {
            case 0:
                r = sin(A * cur_theta) * cos(B * cur_theta);
                break;
            case 1:
                r = sin(A * cos(B * cur_theta));
                break;
            case 2:
                r = cos(A * cos(B * cur_theta));
                break;
            case 3:
                r = sin((A/B) *cur_theta);
                break;
        }
        
        current_cursor.x = cur_theta;
        current_cursor.y = r;
    }
};

struct PolarCVDisplay : LedDisplay {
    PolarCV* module;
    ModuleWidget* moduleWidget;
    int statsFrame = 0;
    std::string fontPath;
    
    Vec current_points[BUFFER_SIZE] = {};
    int currentLayer = 0;
    float prev_settings[3] = {};

    struct Stats {
        float min = INFINITY;
        float max = -INFINITY;
    };
    Stats statsX;
    Stats statsY;

    PolarCVDisplay() {
        fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
    }
    
    void drawEq(const DrawArgs& args, float A, float B, float active_t, Vec current_points[]) {
        if (!module)
            return;

        nvgSave(args.vg);
        Rect b = box.zeroPos().shrink(Vec(0, 15));
        nvgScissor(args.vg, RECT_ARGS(b));
        nvgBeginPath(args.vg);
        
        for (int i = 0; i < BUFFER_SIZE; i++){
            Vec p = current_points[i];
            p = b.interpolate(p);
            if (i == 0)
                nvgMoveTo(args.vg, p.x,  p.y);
            else
                nvgLineTo(args.vg, p.x,  p.y);
        }
        
        nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
        nvgStrokeColor(args.vg, nvgRGB(0xc9, 0xf2, 0xff));
        nvgStrokeWidth(args.vg, 0.3f);
        nvgStroke(args.vg);
        nvgResetScissor(args.vg);
        nvgRestore(args.vg);
    }
    
    void drawCursor(const DrawArgs& args, float A, float B, float active_t, Vec current_cursor) {
        if (!module)
            return;

        nvgSave(args.vg);
        Rect b = box.zeroPos().shrink(Vec(0, 15));
        nvgScissor(args.vg, RECT_ARGS(b));
        nvgBeginPath(args.vg);
        
        Vec p = polarToCart(current_cursor);
        p.x = p.x/2.0f;
        p.y = p.y/2.0f;
        p.x += 0.5f;
        p.y += 0.5f;
        p = b.interpolate(p);
        nvgMoveTo(args.vg, p.x,  p.y-0.12f);
        nvgLineTo(args.vg, p.x-0.08f,  p.y-0.12f);
        nvgLineTo(args.vg, p.x-0.12f,  p.y);
        nvgLineTo(args.vg, p.x-0.04f,  p.y+0.12f);
        nvgLineTo(args.vg, p.x+0.04f,  p.y+0.12f);
        nvgLineTo(args.vg, p.x+0.12f,  p.y);
        nvgLineTo(args.vg, p.x+0.04f,  p.y-0.12f);
        nvgLineTo(args.vg, p.x,  p.y-0.12f);
        //nvgClosePath(args.vg);
        nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
        nvgStrokeColor(args.vg, nvgRGB(0xFF, 0x00, 0x00));
        nvgStrokeWidth(args.vg, 3.0f);
        nvgStroke(args.vg);
        nvgFillColor(args.vg, nvgRGB(0xFF, 0x00, 0x00));
        nvgFill(args.vg);
        nvgResetScissor(args.vg);
        nvgRestore(args.vg);
        
    }
    
    void drawStats(const DrawArgs& args, Vec pos, const char* title, const Stats& stats, float A, float B, float current_equation_t, float time_mults[], int time_mult,float cur_theta) {
        std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
        if (!font)
            return;
        
        int current_equation = int(current_equation_t);
        
        nvgFontSize(args.vg, 13);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, -2);

        nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x40));
        nvgText(args.vg, pos.x + 6, pos.y + 11, title, NULL);

        nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
        pos = pos.plus(Vec(22, 11));

        std::string text;
        text = "";
        text += std::to_string(time_mults[time_mult]).substr(0, 3) + "x";
        nvgText(args.vg, pos.x, pos.y, text.c_str(), NULL);
        
        text = "f = ";
        switch (current_equation) {
            case 0:
                text += std::to_string((floor((A*2)+0.5)/2)).substr(0, 3) + "sin(" + u8"\u00D8" + ") + " + std::to_string((floor((B*2)+0.5)/2)).substr(0, 3) + "cos(" + u8"\u00D8" + ")";
                break;
            case 1:
                text += "sin("+std::to_string((floor((A*2)+0.5)/2)).substr(0, 3)+"cos(" + std::to_string((floor((B*2)+0.5)/2)).substr(0, 3)  +" " + u8"\u00D8" + "))";
                break;
            case 2:
                text += "cos("+std::to_string((floor((A*2)+0.5)/2)).substr(0, 3)+"cos(" + std::to_string((floor((B*2)+0.5)/2)).substr(0, 3) + " " + u8"\u00D8" + "))";
                break;
            case 3:
                text += "sin("+std::to_string((floor((A*2)+0.5)/2)).substr(0, 3)+"/"+std::to_string((floor((B*2)+0.5)/2)).substr(0, 3)+ " " + u8"\u00D8" + ")";
                break;
        }
        
        nvgText(args.vg, pos.x + 40, pos.y, text.c_str(), NULL);
    }
    
    void drawBackground(const DrawArgs& args) {
        Rect b = box.zeroPos().shrink(Vec(0, 15));

        nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x10));
        for (int i = 0; i < 5; i++) {
            nvgBeginPath(args.vg);

            Vec p;
            p.x = 0.0;
            p.y = float(i) / (5 - 1);
            nvgMoveTo(args.vg, VEC_ARGS(b.interpolate(p)));

            p.x = 1.0;
            nvgLineTo(args.vg, VEC_ARGS(b.interpolate(p)));
            nvgStroke(args.vg);
        }
    }
    
    void updatePoints(float A, float B, float current_equation_t){
        
        int current_equation = int(current_equation_t);
        
        float theta = 0.00f;
        float r;
        float min_repeat_pi;
        switch (current_equation) {
            case 0:
                r = sin(A * theta) * cos(B * theta);
                min_repeat_pi  = 8.0f * M_PI;
                break;
            case 1:
                r = sin(A * cos(B * theta));
                min_repeat_pi  = 4.0f * M_PI;
                break;
            case 2:
                r = cos(A * cos(B * theta));
                min_repeat_pi  = 4.0f * M_PI;
                break;
            case 3:
                r = sin((A/B) *theta);
                min_repeat_pi  = 16.0f * M_PI;
                break;
                
        }
        
        for (int i = 0; i < BUFFER_SIZE; i++){
            Vec tempp;
            tempp.x = theta;
            tempp.y = r;
            Vec p = polarToCart(tempp);
            p.x = p.x/2.0f;
            p.y = p.y/2.0f;
            p.x += 0.5f;
            p.y += 0.5f;
            current_points [i] = p;
            
            theta = min_repeat_pi * (float) i / (BUFFER_SIZE - 1) * M_PI;
            switch (current_equation) {
                case 0:
                    r = sin(A * theta) * cos(B * theta);
                    break;
                case 1:
                    r = sin(A * cos(B * theta));
                    break;
                case 2:
                  r = cos(A * cos(B * theta));
                  break;
                case 3:
                    r = sin((A/B) *theta);
                    break;
            }
        }
    }

    void drawLayer(const DrawArgs& args, int layer) override {
        
        if (layer != 1)
            return;
        
        currentLayer += 1;

        if (!module)
            return;

        float A = module->params[PolarCV::A_DIAL_PARAM].getValue() / 2.0f;
        float B = module->params[PolarCV::B_DIAL_PARAM].getValue() / 2.0f;
        
        
        if (currentLayer % 3 == 0){
            updatePoints(A, B, module->params[PolarCV::CUR_EQ_PARAM].getValue());
        }
        
        drawEq(args, A, B, module->cur_theta, current_points);
        
        drawCursor(args, A, B, module->cur_theta, module->current_cursor);
        
        int cur_time = int(module->params[PolarCV::CUR_TIME_MULT_IDX_PARAM].getValue());
        
        drawStats(args, Vec(0, 0 + 1), "1", statsX, A, B, module->params[PolarCV::CUR_EQ_PARAM].getValue(), module->time_mults, cur_time, module->cur_theta);
    }
    
};

struct PolarCVWidget : ModuleWidget {
	PolarCVWidget(PolarCV* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/PolarCV.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(8.467, 86.36)), module, PolarCV::SPEED_MULT_PARAM, PolarCV::SPEED_MULT_LIGHT));
        addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(33.02, 86.36)), module, PolarCV::PREV_EQ_PARAM, PolarCV::PREV_EQ_LIGHT));
        addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(57.573, 86.36)), module, PolarCV::NEXT_EQ_PARAM, PolarCV::NEXT_EQ_LIGHT));
        
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(24.836, 99.901)), module, PolarCV::SPEED_DIAL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(41.204, 99.901)), module, PolarCV::A_DIAL_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(57.573, 99.901)), module, PolarCV::B_DIAL_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.467, 99.901)), module, PolarCV::SPEED_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.833, 115.147)), module, PolarCV::R_OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.02, 115.147)), module, PolarCV::X_OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(45.212, 115.147)), module, PolarCV::Y_OUT_OUTPUT));

        PolarCVDisplay* display = createWidget<PolarCVDisplay>(mm2px(Vec(0.0, 13.039)));
        display->box.size = mm2px(Vec(66.04, 66.04));
        display->module = module;
        display->moduleWidget = this;
        addChild(display);
	}
};


Model* modelPolarCV = createModel<PolarCV, PolarCVWidget>("PolarCV");
