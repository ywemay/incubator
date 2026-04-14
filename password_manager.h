#ifndef PASSWORD_MANAGER_H
#define PASSWORD_MANAGER_H

#include <Arduino.h>

// Conditional compilation for ESP32 only
#ifdef ESP32
#include <Preferences.h>

class PasswordManager {
private:
    Preferences preferences;
    bool initialized;
    
    // Password configuration
    String stored_password_hash;
    bool password_enabled;
    
    // Session management
    bool session_authenticated;
    unsigned long session_expiry;
    static const unsigned long SESSION_TIMEOUT_MS = 30 * 60 * 1000; // 30 minutes
    
public:
    PasswordManager();
    ~PasswordManager();
    
    // Initialization
    bool begin();
    void end();
    
    // Password management
    bool setPassword(const String& password);
    bool clearPassword();
    bool isPasswordEnabled() const { return password_enabled; }
    
    // Authentication
    bool authenticate(const String& password);
    bool checkSession();
    void invalidateSession();
    
    // Security check (convenience method)
    bool requireAuthentication();
    
    // Configuration persistence
    bool saveConfig();
    bool loadConfig();
    
private:
    // Helper functions
    String hashPassword(const String& password);
    bool verifyPassword(const String& password, const String& hash);
};

#else
// Dummy implementation for non-ESP32 platforms
class PasswordManager {
public:
    PasswordManager() {}
    bool begin() { return true; }
    void end() { /* No-op */ }
    bool setPassword(const String& password) { return false; }
    bool clearPassword() { return false; }
    bool isPasswordEnabled() const { return false; }
    bool authenticate(const String& password) { return true; }
    bool checkSession() { return true; }
    void invalidateSession() { /* No-op */ }
    bool requireAuthentication() { return true; }
    bool saveConfig() { return false; }
    bool loadConfig() { return false; }
};

#endif // ESP32

#endif // PASSWORD_MANAGER_H