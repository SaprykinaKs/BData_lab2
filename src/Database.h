// Database.h
#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <fstream>
#include <optional>
#include <unordered_set>

struct Record {
    int id;               // Уникальный ключ
    std::string name;     // Имя
    int age;              // Возраст
    std::string address;  // Адрес

    std::string serialize() const;
    static std::optional<Record> deserialize(const std::string& line);
};

class Database {
public:
    Database(const std::string& filename);
    ~Database();

    bool create();
    bool load();
    void clear();

    bool addRecord(const Record& record);
    bool deleteRecordByField(const std::string& field, const std::string& value);
    std::vector<Record> findRecordsByField(const std::string& field, const std::string& value);
    bool editRecord(const std::string& id, const std::string& fieldToEdit, const std::string& newValue);
    bool createBackup(const std::string& backupFilename);
    bool restoreFromBackup(const std::string& backupFilename);
    bool exportToXlsx(const std::string& xlsxFilename);

private:
    std::string filename;
    std::vector<Record> records;
    static std::unordered_set<int> uniqueIdSet;
};

#endif // DATABASE_H
