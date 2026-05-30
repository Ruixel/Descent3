// 3DS stub for oeAppDatabase.
// Returns safe defaults for all reads; writes are silently discarded.
// Language always defaults to English; no persistent config on 3DS yet.

#ifndef CTR_DATABASE_H
#define CTR_DATABASE_H

#include <cstring>
#include "appdatabase.h"  // oeAppDatabase base class (safe, no platform deps)

class oeCtrAppDatabase : public oeAppDatabase {
public:
  oeCtrAppDatabase() = default;
  ~oeCtrAppDatabase() override = default;

  bool create_record(const char *) override { return true; }
  bool lookup_record(const char *) override { return true; }

  bool read(const char *, char *entry, int *entrylen) override {
    if (entry && entrylen && *entrylen > 0) entry[0] = '\0';
    return false;
  }
  bool read(const char *, void *entry, int wordsize) override {
    if (entry) memset(entry, 0, wordsize);
    return false;
  }
  bool read(const char *, bool *entry) override {
    if (entry) *entry = false;
    return false;
  }

  bool write(const char *, const char *, int) override { return true; }
  bool write(const char *, int) override { return true; }

  void get_user_name(char *buffer, size_t *size) override {
    if (buffer && size && *size > 0) {
      strncpy(buffer, "Player", *size - 1);
      buffer[*size - 1] = '\0';
    }
  }
};

#endif // CTR_DATABASE_H
