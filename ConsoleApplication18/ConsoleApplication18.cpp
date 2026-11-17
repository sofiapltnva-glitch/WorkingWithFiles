/***********************
* Платонова Софья      *
* Работа с файлами     *
* Вариант 19           *
***********************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace std;

struct HardDrive {
  string manufacturer;
  string model;
  double capacity;
  int spindleSpeed;
  double price;
};

struct Result {
  vector<size_t> selected;
  long double averagePrice = 0;
  size_t cheapest = 0;
  size_t mostExpensive = 0;
};

string trim(const string& text) {
  size_t first = text.find_first_not_of(" \t");
  if (first == string::npos) {
    return "";
  }
  size_t last = text.find_last_not_of(" \t");
  return text.substr(first, last - first + 1);
}

vector<string> splitFields(const string& line) {
  vector<string> fields;
  size_t start = 0;
  while (true) {
    size_t separator = line.find(';', start);
    if (separator == string::npos) {
      fields.push_back(trim(line.substr(start)));
      break;
    }
    fields.push_back(trim(line.substr(start, separator - start)));
    start = separator + 1;
  }
  return fields;
}

bool readDouble(const string& text, double& value) {
  istringstream parser(text);
  char extra;
  if (!(parser >> value) || !isfinite(value)) {
    return false;
  }
  return !(parser >> extra);
}

bool readInt(const string& text, int& value) {
  istringstream parser(text);
  char extra;
  if (!(parser >> value)) {
    return false;
  }
  return !(parser >> extra);
}

bool readDrives(const string& path, vector<HardDrive>& drives, string& error) {
  ifstream input(path);
  if (!input) {
    error = "ERROR input";
    return false;
  }
  vector<HardDrive> temporary;
  string line;
  size_t lineNumber = 0;
  while (getline(input, line)) {
    ++lineNumber;
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    vector<string> fields = splitFields(line);
    if (temporary.size() >= 100 || fields.size() != 5) {
      error = "ERROR line " + to_string(lineNumber);
      return false;
    }
    HardDrive current{};
    if (fields[0].empty() || fields[1].empty() ||
      !readDouble(fields[2], current.capacity) ||
      !readInt(fields[3], current.spindleSpeed) ||
      !readDouble(fields[4], current.price) ||
      current.capacity <= 0 || current.spindleSpeed <= 0 ||
      current.price < 0) {
      error = "ERROR line " + to_string(lineNumber);
      return false;
    }
    current.manufacturer = fields[0];
    current.model = fields[1];
    temporary.push_back(current);
  }
  if (input.bad()) {
    error = "ERROR read";
    return false;
  }
  drives = temporary;
  return true;
}

Result processDrives(const vector<HardDrive>& drives, double minimumCapacity) {
  Result result;
  for (size_t i = 0; i < drives.size(); ++i) {
    if (drives[i].capacity > minimumCapacity) {
      result.selected.push_back(i);
    }
    if (drives[i].price < drives[result.cheapest].price) {
      result.cheapest = i;
    }
    if (drives[i].price > drives[result.mostExpensive].price) {
      result.mostExpensive = i;
    }
  }
  if (!result.selected.empty()) {
    for (size_t i = 0; i < result.selected.size(); ++i) {
      long double price = drives[result.selected[i]].price;
      result.averagePrice += (price - result.averagePrice) / (i + 1);
    }
  }
  return result;
}

void printDrive(ostream& output, const HardDrive& drive) {
  output << drive.manufacturer << ";"
    << drive.model << ";"
    << drive.capacity << ";"
    << drive.spindleSpeed << ";"
    << drive.price << "\n";
}

string makeReport(const vector<HardDrive>& drives,
  const Result& result, double minimumCapacity) {
  ostringstream report;
  report << fixed << setprecision(2);
  report << "Capacity limit (GB): " << minimumCapacity << "\n";
  report << "Selected drives (capacity > limit):\n";
  report << "Manufacturer;Model;Capacity_GB;RPM;Price_RUB\n";
  if (result.selected.empty()) {
    report << "EMPTY\n";
  } else {
    for (size_t index : result.selected) {
      printDrive(report, drives[index]);
    }
  }
  report << "Selected count: " << result.selected.size() << "\n";
  report << "Average price (RUB): ";
  if (result.selected.empty()) {
    report << "NONE\n";
  } else {
    report << result.averagePrice << "\n";
  }
  report << "Cheapest drive:\n";
  if (drives.empty()) {
    report << "NONE\n";
  } else {
    printDrive(report, drives[result.cheapest]);
  }
  report << "Most expensive drive:\n";
  if (drives.empty()) {
    report << "NONE\n";
  } else {
    printDrive(report, drives[result.mostExpensive]);
  }
  return report.str();
}

bool writeReport(const string& path, const string& report, string& error) {
  ofstream output(path);
  if (!output) {
    error = "ERROR output";
    return false;
  }
  output << report;
  output.flush();
  if (!output) {
    error = "ERROR write";
    return false;
  }
  output.close();
  if (!output) {
    error = "ERROR write";
    return false;
  }
  cout << report;
  return true;
}

int main() {
  string inputPath;
  string outputPath;
  string capacityText;
  double minimumCapacity;
  if (!getline(cin, inputPath) || !getline(cin, outputPath) ||
    !getline(cin, capacityText)) {
    cout << "ERROR parameters\n";
    return 1;
  }
  if (inputPath.empty() || outputPath.empty() || inputPath == outputPath ||
    !readDouble(capacityText, minimumCapacity) || minimumCapacity < 0) {
    cout << "ERROR parameters\n";
    return 1;
  }
  vector<HardDrive> drives;
  string error;
  if (!readDrives(inputPath, drives, error)) {
    cout << error << "\n";
    return 1;
  }
  Result result = processDrives(drives, minimumCapacity);
  string report = makeReport(drives, result, minimumCapacity);
  if (!writeReport(outputPath, report, error)) {
    cout << error << "\n";
    return 1;
  }
  return 0;
}