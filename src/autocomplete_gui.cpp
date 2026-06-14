#include "header/autocomplete_gui.h"

AutocompleteGui::AutocompleteGui(NextTokenMarkovChain* autocomplete)
    : autocomplete(autocomplete),
      window(900, 520, "Markov Autocomplete"),
      suggestions_view(10, 10, 880, 330),
      input_view(10, 360, 880, 150) {
    suggestions_view.textsize(16);
    suggestions_view.callback(AutocompleteGui::on_suggestion_clicked, this);

    input_view.textsize(16);
    input_view.when(FL_WHEN_CHANGED);
    input_view.callback(AutocompleteGui::on_input_changed, this);

    window.end();
}

int AutocompleteGui::run() {
    window.show();
    return Fl::run();
}

void AutocompleteGui::on_input_changed(Fl_Widget*, void* data) {
    auto* gui = static_cast<AutocompleteGui*>(data);
    if(gui->ignore_next_input_callback) {
        gui->ignore_next_input_callback = false;
        return;
    }
    gui->refresh_suggestions();
}

void AutocompleteGui::on_suggestion_clicked(Fl_Widget*, void* data) {
    static_cast<AutocompleteGui*>(data)->apply_selected_suggestion();
}

bool AutocompleteGui::ends_with_blank(const icu::UnicodeString& text) const {
    if(text.isEmpty()) {
        return false;
    }
    int32_t i = text.moveIndex32(text.length(), -1);
    UChar32 c = text.char32At(i);
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

void AutocompleteGui::refresh_suggestions() {
    std::string current_text = input_view.value();

    suggestions_view.clear();
    current_suggestions_utf8.clear();

    if(current_text.empty()) {
        return;
    }

    icu::UnicodeString unicode_text = icu::UnicodeString::fromUTF8(current_text);
    bool new_word = ends_with_blank(unicode_text);

    std::vector<icu::UnicodeString> suggestions;
    try {
        suggestions = autocomplete->get_suggestions(unicode_text, new_word);
    } catch(...) {
        return;
    }

    for(int i = 0; i < static_cast<int>(suggestions.size()); ++i) {
        std::string utf8;
        suggestions[i].toUTF8String(utf8);
        current_suggestions_utf8.push_back(utf8);
        suggestions_view.add(utf8.c_str());
    }
}

void AutocompleteGui::apply_selected_suggestion() {
    int selected = suggestions_view.value();
    if(selected <= 0 || selected > static_cast<int>(current_suggestions_utf8.size())) {
        return;
    }

    std::string selected_word = current_suggestions_utf8[selected - 1];
    std::string current_text = input_view.value();

    std::string next_text;
    if(current_text.empty()) {
        next_text = selected_word + " ";
    } else {
        icu::UnicodeString unicode_text = icu::UnicodeString::fromUTF8(current_text);
        if(ends_with_blank(unicode_text)) {
            next_text = current_text + selected_word + " ";
        } else {
            std::size_t last_sep = current_text.find_last_of(" \t\n\r");
            if(last_sep == std::string::npos) {
                next_text = selected_word + " ";
            } else {
                next_text = current_text.substr(0, last_sep + 1) + selected_word + " ";
            }
        }
    }

    input_view.value(next_text.c_str());
    input_view.position(static_cast<int>(next_text.size()));

    ignore_next_input_callback = true;
    refresh_suggestions();
}
