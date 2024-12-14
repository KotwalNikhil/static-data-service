#pragma once

#include <string>
#include <iostream>

enum Type
{
    BID = 0,
    ASK,
    TRADE
};

class MarketData
{
public:
    MarketData() {}

    MarketData(std::string symbol, int64_t unixTimestamp, std::string timestamp, std::string exchange, float price, int64_t size, Type type):
    symbol_(symbol), unixTimestamp_(unixTimestamp), timestamp_(timestamp), exchange_(exchange), price_(price), size_(size), type_(type){}

    bool operator <(const MarketData& marketData) const {
        if(unixTimestamp_ == marketData.unixTimestamp_) return symbol_ > marketData.symbol_;
        return unixTimestamp_ > marketData.unixTimestamp_;
    }

    void print() {
        std::cout<<"Printing MarketData object\n";
        std::cout<<symbol_<<" "<<unixTimestamp_<<" "<<exchange_<<" "<<price_<<" "<<size_<<" "<<type_<<"\n";
    }

    std::string symbol_;
    int64_t unixTimestamp_ ;
    std::string timestamp_;
    std::string exchange_ ;
    float price_;
    int64_t size_;
    Type type_;
};