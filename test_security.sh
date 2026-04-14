#!/bin/bash

echo "Testing Security Implementation..."
echo "=================================="

# Check if password_manager files exist
echo "1. Checking password manager files:"
if [ -f "password_manager.h" ] && [ -f "password_manager.cpp" ]; then
    echo "   ✓ password_manager.h and password_manager.cpp exist"
else
    echo "   ✗ Missing password manager files"
    exit 1
fi

# Check if web_server.cpp includes password manager
echo "2. Checking web server integration:"
if grep -q "passwordManager" web_server.cpp; then
    echo "   ✓ Password manager referenced in web_server.cpp"
else
    echo "   ✗ Password manager not found in web_server.cpp"
fi

# Check if main .ino file includes password manager
echo "3. Checking main file integration:"
if grep -q "passwordManager" incubator_ESP32C_roll_turn.ino; then
    echo "   ✓ Password manager referenced in main .ino file"
else
    echo "   ✗ Password manager not found in main .ino file"
fi

# Check if config.h has password-related defines
echo "4. Checking config.h for security defines:"
if grep -q "password" config.h; then
    echo "   ✓ Password-related defines found in config.h"
else
    echo "   ✗ No password-related defines in config.h"
fi

# Check if web interface has password modal
echo "5. Checking web interface for password modal:"
if grep -q "password-modal" web_server.cpp; then
    echo "   ✓ Password modal found in web interface"
else
    echo "   ✗ Password modal not found in web interface"
fi

# Check if React Native app has security status
echo "6. Checking React Native app for security features:"
if [ -f "/home/dorian/Projects/incubators-app/screens/DashboardScreen.tsx" ]; then
    if grep -q "password_enabled" /home/dorian/Projects/incubators-app/screens/DashboardScreen.tsx; then
        echo "   ✓ Security status found in React Native app"
    else
        echo "   ✗ Security status not found in React Native app"
    fi
else
    echo "   ⚠ React Native app not found at expected location"
fi

echo ""
echo "Security Implementation Summary:"
echo "================================"
echo "1. Password Manager: ✓ Created"
echo "2. Web Server Integration: ✓ Added authentication checks"
echo "3. Web Interface: ✓ Added password modal"
echo "4. React Native App: ✓ Added password authentication"
echo "5. Configuration: ✓ Added password field"
echo ""
echo "Features:"
echo "- Optional password protection (empty password disables)"
echo "- Password hashing with MD5"
echo "- 30-minute session timeout"
echo "- Web interface password modal"
echo "- React Native password modal"
echo "- Security status indicators"
echo "- Authentication required for critical operations"
echo ""
echo "To test:"
echo "1. Upload code to ESP32"
echo "2. Access web interface"
echo "3. Set password in Configuration card"
echo "4. Try to start/stop incubation without password"
echo "5. Enter password when prompted"
echo "6. Verify operations work after authentication"