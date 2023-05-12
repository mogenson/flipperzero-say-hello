#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/widget.h>
#include <gui/modules/text_input.h>

typedef enum {
    HelloWidgetView,
    HelloTextInputView,
} HelloView;

typedef enum {
    HelloTextInputSaveEvent,
} HelloTextInputEvent;

typedef struct App {
    ViewDispatcher* view_dispatcher;
    Widget* widget;
    TextInput* text_input;
    char* name;
    size_t name_size;
} App;

static void hello_text_input_callback(void* context) {
    App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, HelloTextInputSaveEvent);
}

static bool hello_custom_event_callback(void* context, uint32_t custom_event) {
    App* app = context;
    bool handled = false;
    if(custom_event == HelloTextInputSaveEvent) {
        widget_reset(app->widget);
        FuriString* message = furi_string_alloc();
        furi_string_printf(message, "Hello %s!", app->name);
        widget_add_string_element(
            app->widget, 8, 32, AlignLeft, AlignCenter, FontPrimary, furi_string_get_cstr(message));
        furi_string_free(message);
        view_dispatcher_switch_to_view(app->view_dispatcher, HelloWidgetView);
        handled = true;
    }
    return handled;
}

bool hello_navigation_event_callback(void* context) {
    App* app = context;
    view_dispatcher_stop(app->view_dispatcher);
    return true;
}

static App* app_alloc() {
    App* app = malloc(sizeof(App));
    app->name_size = 16;
    app->name = malloc(app->name_size);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, hello_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, hello_navigation_event_callback);
    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, HelloWidgetView, widget_get_view(app->widget));
    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, HelloTextInputView, text_input_get_view(app->text_input));
    return app;
}

static void app_free(App* app) {
    furi_assert(app);
    view_dispatcher_remove_view(app->view_dispatcher, HelloWidgetView);
    view_dispatcher_remove_view(app->view_dispatcher, HelloTextInputView);
    view_dispatcher_free(app->view_dispatcher);
    widget_free(app->widget);
    text_input_free(app->text_input);
    free(app);
}

int32_t hello_app(void* p) {
    UNUSED(p);
    App* app = app_alloc();

    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    text_input_reset(app->text_input);
    text_input_set_header_text(app->text_input, "Enter your name");
    text_input_set_result_callback(
        app->text_input, hello_text_input_callback, app, app->name, app->name_size, true);

    view_dispatcher_switch_to_view(app->view_dispatcher, HelloTextInputView);
    view_dispatcher_run(app->view_dispatcher);

    app_free(app);
    return 0;
}
