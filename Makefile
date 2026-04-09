# --- Variables ---
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
TARGET = sort_visualizer
SRC = main.cpp

# Detect OS
UNAME_S := $(shell uname -s)

# --- macOS Configuration ---
ifeq ($(UNAME_S),Darwin)
APP_NAME = SortVisualizer.app

# SFML via pkg-config or fallback
PKG_CFLAGS = $(shell pkg-config --cflags sfml-graphics)
PKG_LIBS = $(shell pkg-config --libs sfml-graphics)

FALLBACK_CFLAGS = -I/opt/homebrew/include
FALLBACK_LIBS = -L/opt/homebrew/lib -lsfml-graphics -lsfml-window -lsfml-system

ifeq ($(PKG_CFLAGS),)
$(warning "pkg-config not found or SFML not set up. Using fallback paths.")
CXXFLAGS += $(FALLBACK_CFLAGS)
LDFLAGS += $(FALLBACK_LIBS)
else
CXXFLAGS += $(PKG_CFLAGS)
LDFLAGS += $(PKG_LIBS)
endif

# --- macOS Targets ---
all: app

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

app: $(TARGET)
	@echo "Creating $(APP_NAME)..."
	@mkdir -p "$(APP_NAME)/Contents/MacOS"
	@mkdir -p "$(APP_NAME)/Contents/Resources"
	@cp $(TARGET) "$(APP_NAME)/Contents/MacOS/"
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > "$(APP_NAME)/Contents/Info.plist"
	@echo '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' >> "$(APP_NAME)/Contents/Info.plist"
	@echo '<plist version="1.0"><dict>' >> "$(APP_NAME)/Contents/Info.plist"
	@echo '  <key>CFBundleExecutable</key><string>$(TARGET)</string>' >> "$(APP_NAME)/Contents/Info.plist"
	@echo '  <key>CFBundleIdentifier</key><string>com.yourname.sortvisualizer</string>' >> "$(APP_NAME)/Contents/Info.plist"
	@echo '  <key>CFBundleName</key><string>Sort Visualizer</string>' >> "$(APP_NAME)/Contents/Info.plist"
	@echo '  <key>CFBundlePackageType</key><string>APPL</string>' >> "$(APP_NAME)/Contents/Info.plist"
	@echo '  <key>CFBundleSignature</key><string>SVIS</string>' >> "$(APP_NAME)/Contents/Info.plist"
	@echo '  <key>CFBundleVersion</key><string>1.0</string>' >> "$(APP_NAME)/Contents/Info.plist"
	@echo '  <key>NSHighResolutionCapable</key><true/>' >> "$(APP_NAME)/Contents/Info.plist"
	@echo '</dict></plist>' >> "$(APP_NAME)/Contents/Info.plist"
	@echo "✅ macOS app built: $(APP_NAME)"

clean:
	@rm -f $(TARGET)
	@rm -rf $(APP_NAME)
	@echo "🧹 Cleaned macOS build files."

# --- Windows Configuration ---
else ifeq ($(OS),Windows_NT)
TARGET_EXE = $(TARGET).exe
SFML_DIR = C:\SFML
INCLUDE_DIR = -I$(SFML_DIR)\include
LIB_DIR = -L$(SFML_DIR)\lib
LIBS = -lsfml-graphics -lsfml-window -lsfml-system
DLL_DIR = $(SFML_DIR)\bin

all: $(TARGET_EXE) postbuild

$(TARGET_EXE): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIR) -o $(TARGET_EXE) $(SRC) $(LIB_DIR) $(LIBS)
	@echo "✅ Windows executable built: $(TARGET_EXE)"

# Copy required DLLs automatically
postbuild:
	@echo "Copying SFML DLLs..."
	@if exist $(DLL_DIR) ( \
		copy $(DLL_DIR)\sfml-*.dll . >nul \
	) else ( \
		echo "⚠️  SFML DLL directory not found: $(DLL_DIR)" \
	)
	@echo "✅ DLLs copied (if available)."

clean:
	@echo "🧹 Cleaning Windows build..."
	@del /Q $(TARGET_EXE) 2>nul || true
	@del /Q sfml-*.dll 2>nul || true
	@echo "Clean complete."

# --- Unknown OS ---
else
$(error Unsupported platform. This Makefile supports only macOS and Windows.)
endif
