#include "password_manager.h"

#ifdef ESP32
#include <Arduino.h>
#include <mbedtls/md5.h>

PasswordManager::PasswordManager() 
    : initialized(false), password_enabled(false), session_authenticated(false), session_expiry(0) {
}

PasswordManager::~PasswordManager() {
    if (initialized) {
        end();
    }
}

bool PasswordManager::begin() {
    if (initialized) {
        return true;
    }
    
    if (!preferences.begin("incubator_pwd", false)) {
        Serial.println("[Password] Failed to initialize preferences");
        return false;
    }
    
    initialized = true;
    
    // Load configuration
    if (!loadConfig()) {
        Serial.println("[Password] Failed to load configuration, using defaults");
        password_enabled = false;
        stored_password_hash = "";
    }
    
    Serial.printf("[Password] Manager initialized. Password enabled: %s\n", 
                  password_enabled ? "YES" : "NO");
    
    return true;
}

void PasswordManager::end() {
    if (initialized) {
        preferences.end();
        initialized = false;
    }
}

String PasswordManager::hashPassword(const String& password) {
    if (password.length() == 0) {
        return "";
    }
    
    // Simple MD5 hash (for demonstration - in production use stronger hashing)
    unsigned char hash[16];
    mbedtls_md5_context ctx;
    mbedtls_md5_init(&ctx);
    mbedtls_md5_starts(&ctx);
    mbedtls_md5_update(&ctx, (const unsigned char*)password.c_str(), password.length());
    mbedtls_md5_finish(&ctx, hash);
    mbedtls_md5_free(&ctx);
    
    // Convert to hex string
    char hex_hash[33];
    for (int i = 0; i < 16; i++) {
        sprintf(hex_hash + (i * 2), "%02x", hash[i]);
    }
    hex_hash[32] = 0;
    
    return String(hex_hash);
}

bool PasswordManager::verifyPassword(const String& password, const String& hash) {
    if (password.length() == 0 || hash.length() == 0) {
        return false;
    }
    
    String computed_hash = hashPassword(password);
    return computed_hash == hash;
}

bool PasswordManager::setPassword(const String& password) {
    if (!initialized) {
        return false;
    }
    
    if (password.length() == 0) {
        // Empty password means disable security
        return clearPassword();
    }
    
    // Hash the password
    stored_password_hash = hashPassword(password);
    password_enabled = true;
    
    // Invalidate any existing session
    invalidateSession();
    
    // Save configuration
    if (saveConfig()) {
        Serial.println("[Password] Password set successfully");
        return true;
    }
    
    return false;
}

bool PasswordManager::clearPassword() {
    if (!initialized) {
        return false;
    }
    
    stored_password_hash = "";
    password_enabled = false;
    
    // Clear any existing session
    session_authenticated = false;
    session_expiry = 0;
    
    // Save configuration
    if (saveConfig()) {
        Serial.println("[Password] Password cleared, security disabled");
        return true;
    }
    
    return false;
}

bool PasswordManager::authenticate(const String& password) {
    if (!initialized) {
        return false;
    }
    
    // If password is not enabled, always authenticate
    if (!password_enabled) {
        session_authenticated = true;
        session_expiry = millis() + SESSION_TIMEOUT_MS;
        return true;
    }
    
    // Check password
    if (verifyPassword(password, stored_password_hash)) {
        session_authenticated = true;
        session_expiry = millis() + SESSION_TIMEOUT_MS;
        Serial.println("[Password] Authentication successful");
        return true;
    }
    
    Serial.println("[Password] Authentication failed");
    return false;
}

bool PasswordManager::checkSession() {
    if (!initialized) {
        return false;
    }
    
    // If password is not enabled, session is always valid
    if (!password_enabled) {
        return true;
    }
    
    // Check if session is authenticated and not expired
    if (session_authenticated && millis() < session_expiry) {
        // Refresh session expiry
        session_expiry = millis() + SESSION_TIMEOUT_MS;
        return true;
    }
    
    // Session expired or not authenticated
    session_authenticated = false;
    return false;
}

void PasswordManager::invalidateSession() {
    session_authenticated = false;
    session_expiry = 0;
    Serial.println("[Password] Session invalidated");
}

bool PasswordManager::requireAuthentication() {
    return checkSession();
}

bool PasswordManager::saveConfig() {
    if (!initialized) {
        return false;
    }
    
    preferences.putBool("enabled", password_enabled);
    preferences.putString("hash", stored_password_hash.c_str());
    
    return preferences.isKey("enabled") && preferences.isKey("hash");
}

bool PasswordManager::loadConfig() {
    if (!initialized) {
        return false;
    }
    
    password_enabled = preferences.getBool("enabled", false);
    stored_password_hash = preferences.getString("hash", "");
    
    // Validate configuration
    if (password_enabled && stored_password_hash.length() == 0) {
        // Invalid state - password enabled but no hash
        password_enabled = false;
        return false;
    }
    
    return true;
}

#endif // ESP32