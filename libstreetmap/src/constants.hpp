/*
 * Constants needed
 */

/* 
 * File:   constants.hpp
 * Author: maksymo4
 *
 * Created on February 23, 2019, 11:14 PM
 */

#pragma once

#include <unordered_map>
#include <string>

//currently "hard-coded" since menu items are created in glade
#define MAX_SUGGESTIONS 5

const std::unordered_map<std::string, std::string> valid_map_paths {
    std::make_pair("beijing" ,"/cad2/ece297s/public/maps/beijing_china.streets.bin"),
    std::make_pair("cairo"   ,"/cad2/ece297s/public/maps/cairo_egypt.streets.bin"),
    std::make_pair("cape-town", "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin"),
    std::make_pair("golden-horseshoe", "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin"),
    std::make_pair("hamilton", "/cad2/ece297s/public/maps/hamilton_canada.streets.bin"),
    std::make_pair("hong-kong", "/cad2/ece297s/public/maps/hong-kong_china.streets.bin"),
    std::make_pair("iceland", "/cad2/ece297s/public/maps/iceland.streets.bin"),
    std::make_pair("interlaken", "/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin"),
    std::make_pair("london", "/cad2/ece297s/public/maps/london_england.streets.bin"),
    std::make_pair("moscow", "/cad2/ece297s/public/maps/moscow_russia.streets.bin"),
    std::make_pair("new-delhi", "/cad2/ece297s/public/maps/new-delhi_india.streets.bin"),
    std::make_pair("new-york", "/cad2/ece297s/public/maps/new-york_usa.streets.bin"),
    std::make_pair("rio", "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin"),
    std::make_pair("saint-helena", "/cad2/ece297s/public/maps/saint-helena.streets.bin"),
    std::make_pair("singapore", "/cad2/ece297s/public/maps/singapore.streets.bin"),
    std::make_pair("sydney", "/cad2/ece297s/public/maps/sydney_australia.streets.bin"),
    std::make_pair("tehran", "/cad2/ece297s/public/maps/tehran_iran.streets.bin"),
    std::make_pair("tokyo", "/cad2/ece297s/public/maps/tokyo_japan.streets.bin"),
    std::make_pair("toronto", "/cad2/ece297s/public/maps/toronto_canada.streets.bin")
};

const std::string HELP_TEXT =
    "Pick between transit and cycling data in the layers menu on the right.\n\n"
    "Click and drag to move around the map and scroll to zoom. Or use the navigation\n"
    "buttons on the right.\n\n"
    "Search for intersections by typing in \"street 1 & street 2\" and then pressing\n"
    "the Find button. Hit the \"enter\" key along the way for suggestions.\n\n"
    "Change maps by typing the name of the new map in the search bar and then pressing\n"
    "enter. See what maps are available by typing \"map-list\"."
;

