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
    std::make_pair("Beijing" ,"/cad2/ece297s/public/maps/beijing_china.streets.bin"),
    std::make_pair("Cairo"   ,"/cad2/ece297s/public/maps/cairo_egypt.streets.bin"),
    std::make_pair("Cape-Town", "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin"),
    std::make_pair("Golden-Horseshoe", "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin"),
    std::make_pair("Hamilton", "/cad2/ece297s/public/maps/hamilton_canada.streets.bin"),
    std::make_pair("Hong-Kong", "/cad2/ece297s/public/maps/hong-kong_china.streets.bin"),
    std::make_pair("Iceland", "/cad2/ece297s/public/maps/iceland.streets.bin"),
    std::make_pair("Interlaken", "/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin"),
    std::make_pair("London", "/cad2/ece297s/public/maps/london_england.streets.bin"),
    std::make_pair("Moscow", "/cad2/ece297s/public/maps/moscow_russia.streets.bin"),
    std::make_pair("New-Delhi", "/cad2/ece297s/public/maps/new-delhi_india.streets.bin"),
    std::make_pair("New-York", "/cad2/ece297s/public/maps/new-york_usa.streets.bin"),
    std::make_pair("Rio", "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin"),
    std::make_pair("Saint-Helena", "/cad2/ece297s/public/maps/saint-helena.streets.bin"),
    std::make_pair("Singapore", "/cad2/ece297s/public/maps/singapore.streets.bin"),
    std::make_pair("Sydney", "/cad2/ece297s/public/maps/sydney_australia.streets.bin"),
    std::make_pair("Tehran", "/cad2/ece297s/public/maps/tehran_iran.streets.bin"),
    std::make_pair("Tokyo", "/cad2/ece297s/public/maps/tokyo_japan.streets.bin"),
    std::make_pair("Toronto", "/cad2/ece297s/public/maps/toronto_canada.streets.bin")
};

