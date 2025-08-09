#include "crow_all.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>
#include <string>
#include <fstream> 

// Dummy sha256 function (replace with real one later)
std::string sha256(const std::string& input) {
    return input; // Just returns input for now
}

// Function to read HTML file
std::string readHTMLFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "<html><body><h1>Frontend file not found. Please copy your frontend.html to index.html in the same folder as your exe file.</h1></body></html>";
    }

    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    file.close();
    return content;
}

int main() {
    try {
        // Database setup
        SQLite::Database db("faces.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        db.exec(R"(CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            face_encoding BLOB
        );)");

        // Create Crow app
        crow::SimpleApp app;

        // Global CORS middleware for all routes
        CROW_ROUTE(app, "/").methods("OPTIONS"_method)
            ([]() {
            crow::response res;
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type");
            return res;
                });

        // CORS for all other routes
        CROW_ROUTE(app, "/<path>").methods("OPTIONS"_method)
            ([](const std::string& path) {
            crow::response res;
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type");
            return res;
                });

        // Root route - serve the frontend HTML
        CROW_ROUTE(app, "/")([]() {
            std::string html_content = readHTMLFile("index.html");
            crow::response res(html_content);
            res.add_header("Content-Type", "text/html");
            return res;
            });

        // Serve index.html explicitly
        CROW_ROUTE(app, "/index.html")([]() {
            std::string html_content = readHTMLFile("index.html");
            crow::response res(html_content);
            res.add_header("Content-Type", "text/html");
            return res;
            });

        // API status route
        CROW_ROUTE(app, "/status")([]() {
            crow::response res("Face Attendance System API Running!");
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Content-Type", "text/plain");
            return res;
            });

        // Login route with CORS
        CROW_ROUTE(app, "/login").methods("POST"_method)
            ([&db](const crow::request& req) {
            auto input = crow::json::load(req.body);
            if (!input || !input.has("email") || !input.has("password")) {
                crow::response res(400, "Invalid JSON or missing fields");
                res.add_header("Access-Control-Allow-Origin", "*");
                res.add_header("Content-Type", "application/json");
                return res;
            }

            std::string email = input["email"].s();
            std::string password = input["password"].s();
            std::string password_hash = sha256(password);

            try {
                SQLite::Statement query(db, "SELECT id FROM users WHERE email = ? AND password_hash = ?;");
                query.bind(1, email);
                query.bind(2, password_hash);

                crow::json::wvalue res_json;
                if (query.executeStep()) {
                    int user_id = query.getColumn(0).getInt();
                    res_json["success"] = true;
                    res_json["user_id"] = user_id;
                }
                else {
                    res_json["success"] = false;
                    res_json["message"] = "Invalid email or password";
                }

                crow::response res{ res_json };
                res.add_header("Access-Control-Allow-Origin", "*");
                res.add_header("Content-Type", "application/json");
                return res;

            }
            catch (const std::exception& e) {
                crow::json::wvalue res_json;
                res_json["success"] = false;
                res_json["message"] = e.what();
                crow::response res{ res_json };
                res.add_header("Access-Control-Allow-Origin", "*");
                res.add_header("Content-Type", "application/json");
                return res;
            }
                });

        // Register route with CORS
        CROW_ROUTE(app, "/register").methods("POST"_method)
            ([&db](const crow::request& req) {
            auto input = crow::json::load(req.body);
            if (!input || !input.has("email") || !input.has("password")) {
                crow::response res(400, "Invalid JSON or missing fields");
                res.add_header("Access-Control-Allow-Origin", "*");
                res.add_header("Content-Type", "application/json");
                return res;
            }

            std::string email = input["email"].s();
            std::string password = input["password"].s();
            std::string password_hash = sha256(password);

            try {
                SQLite::Statement query(db, "INSERT INTO users (email, password_hash) VALUES (?, ?);");
                query.bind(1, email);
                query.bind(2, password_hash);
                query.exec();

                crow::json::wvalue res_json;
                res_json["success"] = true;
                res_json["message"] = "User registered successfully";

                crow::response res{ res_json };
                res.add_header("Access-Control-Allow-Origin", "*");
                res.add_header("Content-Type", "application/json");
                return res;

            }
            catch (const std::exception& e) {
                crow::json::wvalue res_json;
                res_json["success"] = false;
                res_json["message"] = e.what();

                crow::response res{ res_json };
                res.add_header("Access-Control-Allow-Origin", "*");
                res.add_header("Content-Type", "application/json");
                return res;
            }
                });

        // Face verification endpoint
        CROW_ROUTE(app, "/verify-face").methods("POST"_method)
            ([&db](const crow::request& req) {
            auto input = crow::json::load(req.body);

            if (!input || !input.has("user_id")) {
                crow::response res(400, "Missing user_id");
                res.add_header("Access-Control-Allow-Origin", "*");
                res.add_header("Content-Type", "application/json");
                return res;
            }

            int user_id = input["user_id"].i();

            // TODO: Add OpenCV face recognition logic here
            // For now, simulate successful verification

            try {
                crow::json::wvalue res_json;
                res_json["success"] = true;
                res_json["message"] = "Face verification successful! Attendance marked.";
                res_json["user_id"] = user_id;
                res_json["timestamp"] = "2025-08-08 18:58:00";

                crow::response res{ res_json };
                res.add_header("Access-Control-Allow-Origin", "*");
                res.add_header("Content-Type", "application/json");
                return res;

            }
            catch (const std::exception& e) {
                crow::json::wvalue res_json;
                res_json["success"] = false;
                res_json["message"] = e.what();

                crow::response res{ res_json };
                res.add_header("Access-Control-Allow-Origin", "*");
                res.add_header("Content-Type", "application/json");
                return res;
            }
                });

        // Start server
        std::cout << "Starting Face Attendance System server at http://localhost:8080" << std::endl;
        std::cout << "Available endpoints:" << std::endl;
        std::cout << "  GET  / - Frontend interface" << std::endl;
        std::cout << "  GET  /status - API status" << std::endl;
        std::cout << "  POST /login - User login" << std::endl;
        std::cout << "  POST /register - User registration" << std::endl;
        std::cout << "  POST /verify-face - Face verification" << std::endl;
        std::cout << "\nIMPORTANT: Copy your frontend.html to index.html in the same folder!" << std::endl;

        app.port(8080).multithreaded().run();

    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
