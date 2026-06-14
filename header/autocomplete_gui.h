#pragma once
#ifndef AUTOCOMPLETE_GUI_H
#define AUTOCOMPLETE_GUI_H

#include <string>
#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Multiline_Input.H>

#include "header/nexttoken_markov_chain.h"

class AutocompleteGui {
    public:
        explicit AutocompleteGui(NextTokenMarkovChain* autocomplete);
        int run();

    private:
        static void on_input_changed(Fl_Widget*, void* data);
        static void on_suggestion_clicked(Fl_Widget*, void* data);

        bool ends_with_blank(const icu::UnicodeString& text) const;
        void refresh_suggestions();
        void apply_selected_suggestion();

        NextTokenMarkovChain* autocomplete;
        Fl_Double_Window window;
        Fl_Hold_Browser suggestions_view;
        Fl_Multiline_Input input_view;
        std::vector<std::string> current_suggestions_utf8;
        bool ignore_next_input_callback = false;
};

#endif
