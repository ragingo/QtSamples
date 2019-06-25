#pragma once
#include <vector>
#include <QString>

struct SearchResult
{
    struct Meta
    {
        int status = 0;
        int totalCount = 0;
    };

    struct Item
    {
        QString contentId;
        QString title;
        int viewCounter = 0;
        QString thumbnailUrl;

        Item(QString contentId, QString title, int viewCounter, QString thumbnailUrl)
            : contentId(contentId)
            , title(title)
            , viewCounter(viewCounter)
            , thumbnailUrl(thumbnailUrl)
        {}
    };

    struct Data
    {
        std::vector<Item> items;
    };

    SearchResult() {
    }
    SearchResult(const SearchResult& other) {
        meta = other.meta;
        data = other.data;
    }
    SearchResult(SearchResult&& other) noexcept {
        *this = std::move(other);
    }
    SearchResult& operator=(const SearchResult& other) {
        meta = other.meta;
        data = other.data;
        return *this;
    }
    SearchResult& operator=(SearchResult&& other) {
        meta = std::move(other.meta);
        data = std::move(other.data);
        return *this;
    }

    Meta meta = {};
    Data data = {};
};
