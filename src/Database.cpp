#include "Database.h"
#include <sstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <optional>
#include <unordered_set>
#include <xlsxwriter.h>

namespace fs = std::filesystem;

std::string Record::serialize() const {
    std::ostringstream oss;
    oss << id << "," << name << "," << age << "," << address;
    return oss.str();
}

std::optional<Record> Record::deserialize(const std::string& line) {
    std::istringstream ss(line);
    Record record;
    std::string idStr, ageStr;

    if (std::getline(ss, idStr, ',') &&
        std::getline(ss, record.name, ',') &&
        std::getline(ss, ageStr, ',') &&
        std::getline(ss, record.address)) {
        try {
            record.id = std::stoi(idStr);
            record.age = std::stoi(ageStr);
            return record;
        } catch (const std::exception& e) {
            return std::nullopt; // Ошибка преобразования числа
        }
    }
    return std::nullopt; // Ошибка формата
}

Database::Database(const std::string& filename) : filename(filename) {
    load(); 
}


Database::~Database() {}

bool Database::create() {
    std::ofstream file(filename, std::ios::trunc);
    return file.is_open();
}

bool Database::load() {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        auto recordOpt = Record::deserialize(line);
        if (recordOpt) {
            records.push_back(*recordOpt); // преобразование в std::optional
            uniqueIdSet.insert(recordOpt->id); // добавляем ID в уникальный сет
        }
    }
    return true;
}

void Database::clear() {
    std::ofstream file(filename, std::ios::trunc);
}

bool Database::addRecord(const Record& record) {
    if (!uniqueIdSet.emplace(record.id).second) {
        std::cerr << "Error: Record with ID " << record.id << " already exists.\n";
        return false;
    }
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << record.serialize() << '\n';
        return true;
    }
    return false;
}
bool Database::deleteRecordByField(const std::string& field, const std::string& value) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string tempFilename = filename + ".tmp";
    std::ofstream tempFile(tempFilename);
    if (!tempFile.is_open()) return false;

    bool deleted = false;
    std::string line;
    while (std::getline(file, line)) {
        auto recordOpt = Record::deserialize(line);
        if (recordOpt) {
            const Record& record = *recordOpt;
            bool match = false;

            if (field == "id") {
                match = (std::to_string(record.id) == value);
            } else if (field == "name") {
                match = (record.name == value);
            } else if (field == "age") {
                match = (std::to_string(record.age) == value);
            } else if (field == "address") {
                match = (record.address == value);
            }

            if (match) {
                uniqueIdSet.erase(record.id);  // удаляем id из набора уникальных id
                deleted = true;
            } else {
                tempFile << line << '\n';
            }
        }
    }

    file.close();
    tempFile.close();

    if (deleted) {
        fs::rename(tempFilename, filename);
    } else {
        fs::remove(tempFilename); // удаляем временный файл, если ничего не удалено
    }

    return deleted;
}

std::vector<Record> Database::findRecordsByField(const std::string& field, const std::string& value) {
    std::ifstream file(filename);
    std::vector<Record> results;

    if (!file.is_open()) return results;

    std::string line;
    while (std::getline(file, line)) {
        auto recordOpt = Record::deserialize(line);
        if (recordOpt) {
            const Record& record = *recordOpt;
            bool match = false;

            if (field == "id") {
                match = (std::to_string(record.id) == value);
            } else if (field == "name") {
                match = (record.name == value);
            } else if (field == "age") {
                match = (std::to_string(record.age) == value);
            } else if (field == "address") {
                match = (record.address == value);
            }

            if (match) {
                results.push_back(record);
            }
        }
    }

    return results;
}

bool Database::editRecord(const std::string& id, const std::string& fieldToEdit, const std::string& newValue) {
    auto it = std::find_if(records.begin(), records.end(), [&id](const Record& r) { return r.id == std::stoi(id); });

    if (it == records.end()) {  // проверка существования записи
        std::cerr << "Error: Record with ID " << id << " not found." << std::endl;
        return false;
    }

    Record updatedRecord = *it;  // копируем старую запись
    if (fieldToEdit == "name") {
        updatedRecord.name = newValue;
    } else if (fieldToEdit == "age") {
        try {
            updatedRecord.age = std::stoi(newValue);
        } catch (const std::exception& e) {
            std::cerr << "Error: Invalid age value." << std::endl;
            return false;
        }
    } else if (fieldToEdit == "address") {
        updatedRecord.address = newValue;
    } else {
        std::cerr << "Error: Invalid field to edit." << std::endl;
        return false;
    }
    std::ofstream tempFile(filename + ".tmp");
    if (!tempFile.is_open()) {
        std::cerr << "Error: Unable to open temporary file for writing." << std::endl;
        return false;
    }

    bool recordUpdated = false;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        auto recordOpt = Record::deserialize(line);
        if (recordOpt && recordOpt->id == std::stoi(id)) {
            tempFile << updatedRecord.serialize() << '\n';  // записываем обновленную запись
            recordUpdated = true;
        } else {
            tempFile << line << '\n';  
        }
    }

    file.close();
    tempFile.close();

    if (recordUpdated) {
        fs::rename(filename + ".tmp", filename);  // замена старого файла
        return true;
    } else {
        fs::remove(filename + ".tmp");  // удаление временного файла, если запись не обновлена
        return false;
    }
}

bool Database::createBackup(const std::string& backupFilename) {
    fs::copy(filename, backupFilename, fs::copy_options::overwrite_existing);
    return fs::exists(backupFilename);
}

bool Database::restoreFromBackup(const std::string& backupFilename) {
    fs::copy(backupFilename, filename, fs::copy_options::overwrite_existing);
    return fs::exists(filename);
}
bool Database::exportToXlsx(const std::string& xlsxFilename) {
    lxw_workbook* workbook = workbook_new(xlsxFilename.c_str());
    if (!workbook) return false;

    lxw_worksheet* worksheet = workbook_add_worksheet(workbook, NULL);

    // Add headers
    worksheet_write_string(worksheet, 0, 0, "ID", NULL);
    worksheet_write_string(worksheet, 0, 1, "Name", NULL);
    worksheet_write_string(worksheet, 0, 2, "Age", NULL);
    worksheet_write_string(worksheet, 0, 3, "Address", NULL);

    // Write records
    std::ifstream file(filename);
    if (!file.is_open()) {
        workbook_close(workbook);
        return false;
    }

    int row = 1;
    std::string line;
    while (std::getline(file, line)) {
        auto recordOpt = Record::deserialize(line);
        if (recordOpt) {
            const Record& record = *recordOpt;
            worksheet_write_number(worksheet, row, 0, record.id, NULL);         // ID
            worksheet_write_string(worksheet, row, 1, record.name.c_str(), NULL);  // Name
            worksheet_write_number(worksheet, row, 2, record.age, NULL);          // Age
            worksheet_write_string(worksheet, row, 3, record.address.c_str(), NULL);  // Address
            row++;
        }
    }

    file.close();
    workbook_close(workbook);
    return true;
}
std::unordered_set<int> Database::uniqueIdSet;  