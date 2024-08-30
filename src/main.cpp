#include "main.h"

fs::path json_directory{"japi\\merging\\param\\battle\\SpeakingLineParam"};
JSON json_data;

typedef u64*(__fastcall* Parse_SpeakingLineParam_t)(u64*);
Parse_SpeakingLineParam_t Parse_SpeakingLineParam_original;

u64* __fastcall Parse_SpeakingLineParam(u64* a1) {
    u64* result;            // To be returned at end of function.

    struct Input_Data {
        kojo::binary data;          // Data from XFBIN chunk, exactly as-is.
        u32 entry_count;            // Number of entries.
        u64 note_pointer;           // Initial pointer, pointing to start of entries. Can be used to skip over notes.
    } input;

    struct Entry {

    };  // All data belonging to only one entry.
    std::map<std::string, Entry> entries;

    // Load data from XFBIN file.
        input.data.load(Load_nuccBinary("data/param/battle/SpeakingLineParam.bin.xfbin", "SpeakingLineParam"));
        if (!input.data.data()) {
            JERROR("`SpeakingLineParam.bin.xfbin` data could not be loaded.");
            return 0;
        }
        JTRACE("Loaded SpeakingLineParam data from XFBIN.");

        input.data.change_pos(4); // Version (should be 1000)
        input.entry_count = input.data.read<u32>(kojo::endian::little);
        JTRACE("Entry Count from XFBIN: {}", input.entry_count);
        input.note_pointer = input.data.read<u64>(kojo::endian::little); // 8 for all vanilla files.
        if (input.note_pointer == 0) {
            JERROR("`SpeakingLineParam.bin.xfbin` is missing an initial pointer.");
            return 0;
        }

    // Iterate through each entry.
        for (int i = 0; i < input.entry_count; i++) {
            
        }
    
    // Load data from JSON files.
        for (const auto& item : json_data.items()) {
            
        }

    // Load all data into game.
        for (const auto& [key, value] : entries) {
            
        }

    return result;
}

Hook Parse_SpeakingLineParam_hook = {
    (void*)0x23E464, // Address of the function we want to hook
    (void*)Parse_SpeakingLineParam, // Address of our hook function
    (void**)&Parse_SpeakingLineParam_original, // Address of the variable that will store the original function address
    "Parse_SpeakingLineParam" // Name of the function we want to hook
};

// This function is called when the mod is loaded.
void __stdcall ModInit() {
    if (!JAPI_HookASBRFunction(&Parse_SpeakingLineParam_hook))
        JERROR("Failed to hook function `{}`.", Parse_SpeakingLineParam_hook.name);

    // Create directory for JSON files if not already existing.
    if (!fs::exists(json_directory)) {
        JDEBUG("Attempting to create directory...");
        fs::create_directories(json_directory);
    }
    if (!fs::exists(json_directory)) JFATAL("Failed to create directory at:\n{}", json_directory.string());

    fs::path priority_path{json_directory / "_priority.json"};
    JSON priority_json;

    // Read existing priority data, if it exists.
    if (fs::exists(priority_path)) {
        std::ifstream priority_file(priority_path);
        priority_json = JSON::parse(priority_file);
        priority_file.close();
    }

    // Refresh priority data.
    for (const auto& entry : fs::directory_iterator(json_directory)) {
        if (entry.path().extension() == ".json" && entry.path().filename() != "_priority.json")
            if (priority_json[entry.path().filename().stem().string()] == nullptr)
                priority_json[entry.path().filename().stem().string()] = 0;
    }
    for (auto& item : priority_json.items()) {
        if (!fs::exists(json_directory / (item.key() + ".json")))
            priority_json.erase(item.key());
    }

    // Create new priority JSON file.
    std::ofstream priority_file(priority_path);
    priority_file << priority_json.dump(2);
    priority_file.close();

    // Merge JSON files together.
    std::map<int, std::vector<std::string>> priority_list;
    std::ifstream json_file_buffer;
    JSON json_buffer;
    std::string filename_buffer;
    for (auto& item : priority_json.items()) {
        priority_list[item.value()].push_back(item.key());
    }
    for (const auto& [key, value] : priority_list) {
        for (const auto& str : value) {
            filename_buffer = str + ".json";
            json_file_buffer.open(json_directory / filename_buffer);
            if (!json_file_buffer.is_open()) {
                JWARN("{} could not be opened.", filename_buffer);
                continue;
            }

            try {
                json_buffer = JSON::parse(json_file_buffer);
                json_data.merge_patch(json_buffer);
            } catch (const JSON::parse_error& e) {
                JWARN("{}: {}", filename_buffer, e.what());
            }

            json_file_buffer.close();
        }
    }

    JINFO("Loaded!");
}