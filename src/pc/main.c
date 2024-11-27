#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "./error_codes.h"
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "./uart_pc.h"

#define TRACK_MAX_SIZE 4096
#define TRACK_MAX_LEN 64
#define INSTRUMENT_MAX_LEN 64

#define META_EVENT 0xFF
#define SYSEX_EVENT_START 0xF0
#define SYSEX_EVENT_RESTART 0xF7

#define MIDI_NOTE_OFF 0x80
#define MIDI_NOTE_ON 0x90
#define MIDI_CONTROL_CHANGE 0xB0

#define META_TRACK_NAME 0x03
#define META_INSTRUMENT_NAME 0x04
#define META_TRACK_END 0x2F
#define META_SET_TEMPO 0x51
#define META_TIME_SIGNATURE 0x58
#define META_KEY_SIGNATURE 0x59


FILE* out_file;
uint8_t out_file_path[] = ".\\out.mxy";

typedef enum notes {
    DELAY = 0,
    C1 = 1,
    D1 = 2,
    E1 = 3,
    F1 = 4,
    G1 = 5,
    A1 = 6,
    B1 = 7,
    C2 = 8,
    END = 0xFF
} notes_e;

#define MIDI_C1 60
#define MIDI_D1 62
#define MIDI_E1 64
#define MIDI_F1 65
#define MIDI_G1 67
#define MIDI_A1 69
#define MIDI_B1 71
#define MIDI_C2 72

#define N_TRACKS 1



struct MIDI_INFO
{
    uint16_t file_format;
    uint16_t n_tracks;
    uint16_t ticks_per_quarter;
} midi_info;

typedef struct track_info
{
    long start_track;
    long current_track_pos;
    long track_len;
    uint8_t parsed_track[TRACK_MAX_SIZE];
    uint16_t parsed_track_info;
    uint8_t last_command;
    uint8_t instrument[INSTRUMENT_MAX_LEN];
    uint8_t track_name[TRACK_MAX_LEN];
    uint32_t track_tempo_us;
    uint32_t time_signature;
    uint16_t key_signature;
    uint16_t ticks_per_quarter;
} track_info_t;

bool verify_header(FILE* file);

int32_t get_track_pointers(FILE* midi_file, track_info_t* pointers, uint8_t n_ptrs);

void read_track(FILE* midi_file, track_info_t* track, FILE* out_file);

int16_t read_command(FILE* midi_file, track_info_t* track, FILE* out_file);

void create_delay(int64_t delta_time, track_info_t* track, FILE* out_file);

int64_t read_delta_time(FILE* midi_file, track_info_t* track);

int64_t read_var_len(FILE* file, track_info_t* track);

int16_t read_byte(FILE* file, track_info_t* track);

int16_t parse_command(FILE* file, track_info_t* track, uint8_t data_byte_1, FILE* out_file);

int16_t parse_meta_event(FILE* file, track_info_t* track, uint8_t meta_command, FILE* out_file);

int16_t parse_midi_event(FILE* file, track_info_t* track, uint8_t data_byte_1, FILE* out_file);

void create_note(notes_e note, uint8_t velocity, FILE* out_file);

uint64_t hex2uint(const char* hexstr, uint8_t hexstr_len);



int main(int argc, uint8_t* argv[])
{
    ///////////////////////
    // Convert MIDI file //
    ///////////////////////
    if (argc != 3)
    {
        fprintf(stderr, "Insufficient arguments: %i, expected 2", argc - 1);
        return ERROR_ARGS;
    }
    uint8_t* path = argv[1];
    uint8_t COM = strtol(argv[2], NULL, 10);

    printf("Loading midi file...\n");
    // char pwd[1024];
    // getcwd(pwd, 1024);
    // printf("cwd: %s\n", pwd);
    // FILE* f = fopen("C:\\Users\\mgaho\\Documents\\Martin\\School\\VUT\\5_semestr\\DE2\\project\\Xylophone\\src\\pc\\test.mid", "rb");
    FILE* f = fopen(path, "rb");
    if (f == NULL)
    {
        fprintf(stderr, "Failed to load midi file: %i.\n", errno);
        return ERROR_LOADING_FILE;
    }
    if (!verify_header(f)) // Verify MIDI file header
    {
        fprintf(stderr, "Header error.\n");
        fclose(f); // Close MIDI file
        return ERROR_MIDI_HEADER;
    }

    track_info_t tracks[N_TRACKS];
    if (get_track_pointers(f, tracks, N_TRACKS)) // Read single track
    {
        fprintf(stderr, "Error getting track starting positions.\n");
        fclose(f); // Close MIDI file
        return ERROR_POS;
    }

    // Create output file
    out_file = fopen(out_file_path, "wb");
    if (out_file == NULL)
    {
        fprintf(stderr, "Error creating output file.\n");
        fclose(f);
        return ERROR_LOADING_FILE;
    }


    // Do stuff
    read_track(f, &tracks[0], out_file);


    fclose(f); // Close MIDI file



    //////////////////////
    // Send parsed midi //
    //////////////////////

    printf("Upload result: %i", uart_send(out_file, 1));

    fclose(out_file); // Close output file

    return SUCCESS;
}

bool verify_header(FILE* file)
{
    uint8_t buffer[4];
    uint32_t header_len;
    if (fread(buffer, 1, 4, file) < 4) return false; // Read header start bytes
    if (memcmp(buffer, "MThd", 4)) return false; // Verify header bytes
    if (fread(buffer, 1, 4, file) < 4) return false; // Read header len
    header_len = hex2uint(buffer, 4);
    if (header_len != 6) return false; // Verify header len
    if (fread(buffer, 1, 2, file) < 2) return false; // Get file format
    midi_info.file_format = hex2uint(buffer, 2);
    if (fread(buffer, 1, 2, file) < 2) return false; // Get number of tracks
    midi_info.n_tracks = hex2uint(buffer, 2);
    if (fread(buffer, 1, 2, file) < 2) return false; // Get ticks per quarter note
    midi_info.ticks_per_quarter = hex2uint(buffer, 2);
    return true;
}

uint64_t hex2uint(const char* hexstr, uint8_t hexstr_len)
{
    uint64_t result = 0;
    if (hexstr_len > 4) return 0; // Safety feature
    for (int i = 0; i < hexstr_len; i++) // Big endian convert - for some reason does not work in a single command
    {
        uint8_t byte = hexstr[i];
        uint8_t shift = 8 * (hexstr_len - i - 1);
        uint64_t value_shift = byte << shift;
        result += value_shift;
    }
    return result;
}

int32_t get_track_pointers(FILE* midi_file, track_info_t* pointers, uint8_t n_ptrs)
{
    if (n_ptrs > midi_info.n_tracks) // Check number of valid tracks
    {
        fprintf(stderr, "Error - expected number of tracks: %i, number of tracks in file: %i\n", n_ptrs, midi_info.n_tracks);
        return ERROR_NTRACKS;
    }
    // Next should be track identifier, contains length -> offset for the next track start
    uint8_t str[4];
    uint32_t track_len;
    for (int i = 0; i < n_ptrs; i++)
    {
        // if (f scanf(midi_file, "%4s", str) == EOF) return ERROR_FILE_EOF; // Read 4 bytes of track header
        if (fread(str, 1, 4, midi_file) < 4) return ERROR_FILE_EOF; // Read 4 bytes of track header
        if (memcmp(str, "MTrk", 4)) return ERROR_POS; // verify position
        // if (f scanf(midi_file, "%4s", str) == EOF) return ERROR_FILE_EOF; // Read 4 bytes of track length
        if (fread(str, 1, 4, midi_file) < 4) return ERROR_FILE_EOF; // Read 4 bytes of track length
        track_len = hex2uint(str, 4); // Convert track length
        pointers[i].start_track = ftell(midi_file); // Save start position of the track
        pointers[i].track_len = track_len; // Save track length
        pointers[i].current_track_pos = pointers[i].start_track; // Save current track position
        pointers[i].parsed_track_info = 0; // Zero parsed track array position
        memset(pointers[i].parsed_track, 0x00, TRACK_MAX_SIZE); // Clear parsed track buffer
        fseek(midi_file, track_len, SEEK_CUR); // Move in the file to next track start
    }
    return SUCCESS;
}

void read_track(FILE* midi_file, track_info_t* track, FILE* out_file)
{
    int16_t result = 0;
    while (result != END && result != ERROR_POS) 
    {
        for (int i = 0; i < N_TRACKS; i++)
        {
            result = read_command(midi_file,  &track[i], out_file);
        }
    }
    printf("Parsing result: %d\n", result == END ? SUCCESS : result);
}

int16_t read_command(FILE* midi_file, track_info_t* track, FILE* out_file)
{
    int64_t delta_time;
    if ((delta_time = read_delta_time(midi_file, track)) < 0)
    {
        fprintf(stderr, "Failed to read delta time at position %lli\n", track->current_track_pos);
        return ERROR_POS;
    }
    if (delta_time != 0) // Save delta time if needed as delay
    {
        create_delay(delta_time, track, out_file);
    }
    uint8_t data;
    if ((data = read_byte(midi_file, track)) < 0)
    {
        fprintf(stderr, "Failed to read byte at position %lli\n", track->current_track_pos);
        return ERROR_POS;
    }
    if (data & 0x80) // If command
    {
        track->last_command = data; // Update command
        if ((data = read_byte(midi_file, track)) < 0) // Read next byte
        {
            fprintf(stderr, "Failed to read byte at position %lli\n", track->current_track_pos);
            return ERROR_POS;
        }
    }
    int16_t result = parse_command(midi_file, track, data, out_file);
    
}

void create_delay(int64_t delta_time, track_info_t* track, FILE* out_file)
{
    uint16_t delay_ms = delta_time * track->track_tempo_us / 1000 / track->ticks_per_quarter; // Convert to ms
    fprintf(out_file, "%c%c%c", DELAY, delay_ms >> 8, delay_ms & 0x00FF); // Write bytes to output file
    return;
}

int64_t read_delta_time(FILE* midi_file, track_info_t* track)
{
    return read_var_len(midi_file, track);
}

int64_t read_var_len(FILE* file, track_info_t* track)
{
    uint8_t c;
    uint32_t value;
    long pos = ftell(file); // Save current position
    fseek(file, track->current_track_pos, SEEK_SET); // Set position to current track position
    // if (f scanf(file, "%c", &c) == EOF) return ERROR_FILE_EOF; // Read byte
    if (fread(&c, 1, 1, file) < 1) return ERROR_FILE_EOF; // Read byte
    value = c & 0x7F; // Save value
    if (c & 0x80) // If part of variable length value
    {
        do
        {
            value <<= 7; // Shift by 7 bits
            // if (f scanf(file, "%c", &c) == EOF) return ERROR_FILE_EOF; // Read next byte
            if (fread(&c, 1, 1, file) < 1) return ERROR_FILE_EOF; // Read next byte
            value += (c & 0x7F); // Add byte
        } while (c & 0x80); // While next byte is part of the current value
    }
    track->current_track_pos = ftell(file); // Save new track current position
    fseek(file, pos, SEEK_SET); // Reset file position
    return (int64_t)value; // Return read variable length value
}

int16_t read_byte(FILE* file, track_info_t* track)
{
    uint8_t c;
    long pos = ftell(file);
    fseek(file, track->current_track_pos, SEEK_SET);
    // if (f scanf(file, "%c", &c) == EOF) return ERROR_POS; // Get single byte
    if (fread(&c, 1, 1, file) < 1) return ERROR_FILE_EOF; // Get single byte
    track->current_track_pos = ftell(file);
    fseek(file, pos, SEEK_SET);
    return (int16_t)c;
}

int16_t parse_command(FILE* file, track_info_t* track, uint8_t data_byte_1, FILE* out_file)
{
    int16_t result;
    switch (track->last_command)
    {
        case META_EVENT: // META event
        {
            result = parse_meta_event(file, track, data_byte_1, out_file); // Parse META event
            break;
        }
        case SYSEX_EVENT_RESTART:
        case SYSEX_EVENT_START:
        {
            return ERROR_UNKNOWN_EVENT; // SysEX events not supported
            break;
        }
        default: // MIDI event (possibly)
        {
            result = parse_midi_event(file, track, data_byte_1, out_file); // Parse MIDI event
            break;
        }
    }
    return result;
}

int16_t parse_meta_event(FILE* file, track_info_t* track, uint8_t meta_command, FILE* out_file)
{
    uint8_t len = read_byte(file, track); // Read length of meta event data
    uint8_t data[len]; // Create data buffer
    for (int i = 0; i < len; i++)
    {
        int16_t value = read_byte(file, track);
        if (value < 0) return value;
        data[i] = value; // Read all bytes for the current meta event
    }
    switch(meta_command)
    {
        case META_TRACK_NAME: // Instrument name
        {
            strncpy(track->track_name, data, TRACK_MAX_LEN - 1); // Copy instrument name to track info
            break;
        }
        case META_INSTRUMENT_NAME: // Instrument name
        {
            strncpy(track->instrument, data, INSTRUMENT_MAX_LEN - 1); // Copy instrument name to track info
            break;
        }
        case META_KEY_SIGNATURE: // Key signature
        {
            if (len != 2) return ERROR_INCORRECT_EVENT_LENGTH;
            track->key_signature = (data[0] << 8) | data[1]; // Save key signature in form SF MI
            break;
        }
        case META_SET_TEMPO: // Tempo
        {
            if (len != 3) return ERROR_INCORRECT_EVENT_LENGTH;
            track->track_tempo_us = (data[0] << 16) | (data[1] << 8) | data[2]; // Save tempo in us for delay creation
            break;
        }
        case META_TIME_SIGNATURE: // Time signature
        {
            if (len != 4) return ERROR_INCORRECT_EVENT_LENGTH;
            track->time_signature = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]; // Save time signature NN DD CC BB
            break;
        }
        case META_TRACK_END: // Track end symbol
        {
            if (len) return ERROR_INCORRECT_EVENT_LENGTH;
            fprintf(out_file, "%c", (uint8_t)END);
            return (int16_t)END;
        }
        default: return ERROR_UNKNOWN_EVENT;
    }
    return SUCCESS;
}

int16_t parse_midi_event(FILE* file, track_info_t* track, uint8_t data_byte_1, FILE* out_file)
{
    uint8_t command = track->last_command & 0xF0; // Get upper four bits of the command
    switch(command)
    {
        case MIDI_NOTE_OFF: // don't need to support note off for xylophone
        {
            return ERROR_UNKNOWN_EVENT;
        }
        case MIDI_NOTE_ON:
        {
            uint8_t note = data_byte_1;
            uint8_t note_velocity;
            long pos = ftell(file); // Save current position
            fseek(file, track->current_track_pos, SEEK_SET); // Get to last known track position
            // if (f scanf(file, "%c", &note) == EOF) return ERROR_FILE_EOF; // Get another byte
            if (fread(&note_velocity, 1, 1, file) < 1) return ERROR_FILE_EOF; // Get note velocity byte
            track->current_track_pos = ftell(file); // Save new track position
            fseek(file, pos, SEEK_SET); // Restore previous position
            switch(note) // Check on note
            {
                case MIDI_C1: create_note(C1, note_velocity, out_file); break;
                case MIDI_D1: create_note(D1, note_velocity, out_file); break;
                case MIDI_E1: create_note(E1, note_velocity, out_file); break;
                case MIDI_F1: create_note(F1, note_velocity, out_file); break;
                case MIDI_G1: create_note(G1, note_velocity, out_file); break;
                case MIDI_A1: create_note(A1, note_velocity, out_file); break;
                case MIDI_B1: create_note(B1, note_velocity, out_file); break;
                case MIDI_C2: create_note(C2, note_velocity, out_file); break;
                default: return ERROR_UNKNOWN_EVENT;
            }
            break;
        }
        case MIDI_CONTROL_CHANGE:
        {
            return ERROR_UNKNOWN_EVENT; // There is not a single interesting event for this simple project
        }
        default: return ERROR_UNKNOWN_EVENT;
    }
    return SUCCESS;
}

void create_note(notes_e note, uint8_t velocity, FILE* out_file)
{
    if (!velocity) return;
    fprintf(out_file, "%c", note); // Save note to the output file
}



