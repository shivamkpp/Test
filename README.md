# Deribit Trading System

## Overview

This project is a high-performance order execution and management system designed to trade on the Deribit Test platform using C++. It supports trading in Spot, Futures, and Options across all supported symbols. The system is built to ensure low-latency performance and includes comprehensive error handling and logging.

## Features

- **Order Management**:
  - Place, cancel, and modify orders.
  - Retrieve orderbook data.
  - View current trading positions.

- **Real-time Market Data Streaming**:
  - WebSocket server functionality for real-time data distribution.
  - Clients can subscribe to symbols and receive continuous orderbook updates.

- **Market Coverage**:
  - Instruments: Spot, Futures, and Options.
  - Scope: All supported symbols on Deribit Test.

## Installation

### Prerequisites

- C++ compiler (e.g., g++, clang++)
- CMake
- Required libraries: OpenSSL, CURL, Threads, nlohmann_json, jsoncpp

### Build Instructions

1. **Clone the repository**:
   ```bash
   git clone https://github.com/yourusername/deribit-trading-system.git
   ```
2. **Navigate to the project directory**:
   ```bash
   cd deribit-trading-system
   ```
3. **Build the project using CMake**:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

## Usage

1. **Create a Deribit Test account** and generate API keys for authentication.
2. **Run the application**:
   ```bash
   ./deribit_trading [options]
   ```
   Replace `[options]` with any command-line arguments your application supports.

## Dependencies

- **OpenSSL**: For secure communication.
- **CURL**: For HTTP requests.
- **Threads**: For multi-threading support.
- **nlohmann_json**: For JSON parsing.
- **jsoncpp**: For additional JSON handling.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request for any improvements or bug fixes.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact

For questions or feedback, please contact [Your Name] at [Your Email] or open an issue on GitHub.
