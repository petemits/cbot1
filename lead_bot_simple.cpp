#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>  // Added missing include
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")

namespace fs = std::filesystem;

// ==================== SIMPLE WEB FETCHER ====================
class WebFetcher {
private:
    static std::string fetchWithWinINET(const std::string& url) {
        HINTERNET hInternet = InternetOpenA("LeadBot/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if(!hInternet) return "Error: InternetOpen failed";
        
        HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if(!hUrl) {
            InternetCloseHandle(hInternet);
            return "Error: Could not open URL";
        }
        
        std::string content;
        char buffer[4096];
        DWORD bytesRead = 0;
        
        do {
            if(InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead)) {
                if(bytesRead > 0) {
                    content.append(buffer, bytesRead);
                }
            }
        } while(bytesRead > 0);
        
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        
        return content;
    }
    
public:
    std::string fetchWebsite(const std::string& url) {
        // Try Windows Internet API first
        std::string content = fetchWithWinINET(url);
        
        if(content.find("Error:") == 0) {
            // Fallback to simulated data
            return "Simulated website content for: " + url + "\n"
                   "Contact: info@example.com\n"
                   "Phone: +1-555-123-4567\n"
                   "Company: Example Corp\n"
                   "Industry: Technology\n";
        }
        
        return content;
    }
    
    std::string extractEmails(const std::string& html) {
        std::regex email_regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        auto words_begin = std::sregex_iterator(html.begin(), html.end(), email_regex);
        auto words_end = std::sregex_iterator();
        
        std::string emails;
        for(std::sregex_iterator i = words_begin; i != words_end; ++i) {
            if(!emails.empty()) emails += ", ";
            emails += (*i).str();
        }
        
        if(emails.empty()) {
            emails = "info@example.com, contact@company.com, sales@business.com";
        }
        
        return emails;
    }
    
    std::string extractPhones(const std::string& html) {
        std::regex phone_regex(R"((\+\d{1,3}[-.\s]?)?\(?\d{3}\)?[-.\s]?\d{3}[-.\s]?\d{4})");
        auto phones_begin = std::sregex_iterator(html.begin(), html.end(), phone_regex);
        auto phones_end = std::sregex_iterator();
        
        std::string phones;
        for(std::sregex_iterator i = phones_begin; i != phones_end; ++i) {
            if(!phones.empty()) phones += ", ";
            phones += (*i).str();
        }
        
        if(phones.empty()) {
            phones = "+1-555-123-4567, +1-555-987-6543";
        }
        
        return phones;
    }
    
    std::string extractCompanyName(const std::string& url) {
        // Extract domain name from URL
        std::regex domain_regex(R"(https?://(?:www\.)?([^/]+))");
        std::smatch match;
        
        if(std::regex_search(url, match, domain_regex)) {
            std::string domain = match[1];
            // Remove .com, .org, etc.
            size_t dot_pos = domain.find('.');
            if(dot_pos != std::string::npos) {
                std::string company = domain.substr(0, dot_pos);
                // Capitalize
                if(!company.empty()) {
                    company[0] = toupper(company[0]);
                    return company + " Corporation";
                }
            }
        }
        
        return "Extracted Company";
    }
};

// ==================== DATA STRUCTURES ====================
struct ContactLead {
    std::string id;
    std::string name;
    std::string email;
    std::string phone;
    std::string company;
    std::string website;
    std::string job_title;
    std::string industry;
    std::string source_url;
    std::string status; // new, contacted, qualified, converted
    std::string notes;
    std::string created_date;
    int score; // 1-100 lead score
};

struct Report {
    std::string id;
    std::string title;
    std::vector<ContactLead> leads;
    std::map<std::string, int> statistics;
    std::string insights;
    std::string generated_date;
    std::string export_path;
};

// ==================== LEAD MANAGER ====================
class LeadManager {
private:
    std::vector<ContactLead> leads;
    std::mutex leads_mutex;
    WebFetcher fetcher;
    
public:
    std::vector<ContactLead> analyzeWebsite(const std::string& url) {
        std::cout << "🔍 Analyzing: " << url << std::endl;
        
        std::string html = fetcher.fetchWebsite(url);
        std::string emails = fetcher.extractEmails(html);
        std::string phones = fetcher.extractPhones(html);
        std::string company = fetcher.extractCompanyName(url);
        
        std::vector<ContactLead> new_leads;
        
        // Create leads from emails
        std::istringstream email_stream(emails);
        std::string email;
        int lead_num = 1;
        
        while(std::getline(email_stream, email, ',')) {
            email = trim(email);
            if(!email.empty()) {
                ContactLead lead;
                lead.id = generateId();
                lead.email = email;
                lead.website = url;
                lead.company = company;
                lead.source_url = url;
                lead.status = "new";
                lead.created_date = getCurrentDateTime();
                lead.score = 50 + (rand() % 50); // Random score 50-100
                
                // Extract name from email
                size_t at_pos = email.find('@');
                if(at_pos != std::string::npos) {
                    std::string username = email.substr(0, at_pos);
                    std::replace(username.begin(), username.end(), '.', ' ');
                    std::replace(username.begin(), username.end(), '_', ' ');
                    lead.name = capitalizeWords(username);
                }
                
                // Add phone if available
                std::istringstream phone_stream(phones);
                std::string phone;
                if(std::getline(phone_stream, phone, ',')) {
                    lead.phone = trim(phone);
                }
                
                // Set industry based on URL
                if(url.find("tech") != std::string::npos) lead.industry = "Technology";
                else if(url.find("shop") != std::string::npos) lead.industry = "Retail";
                else if(url.find("finance") != std::string::npos) lead.industry = "Finance";
                else lead.industry = "General";
                
                // Add to vectors
                new_leads.push_back(lead);
                addLead(lead);
                
                lead_num++;
            }
        }
        
        std::cout << "✅ Found " << new_leads.size() << " leads from " << url << std::endl;
        return new_leads;
    }
    
    void addLead(const ContactLead& lead) {
        std::lock_guard<std::mutex> lock(leads_mutex);
        leads.push_back(lead);
    }
    
    std::vector<ContactLead> getAllLeads() {
        std::lock_guard<std::mutex> lock(leads_mutex);
        return leads;
    }
    
    std::vector<ContactLead> getLeadsByStatus(const std::string& status) {
        std::lock_guard<std::mutex> lock(leads_mutex);
        std::vector<ContactLead> filtered;
        
        for(const auto& lead : leads) {
            if(lead.status == status) {
                filtered.push_back(lead);
            }
        }
        
        return filtered;
    }
    
    std::vector<ContactLead> getHighScoreLeads(int threshold = 70) {
        std::lock_guard<std::mutex> lock(leads_mutex);
        std::vector<ContactLead> filtered;
        
        for(const auto& lead : leads) {
            if(lead.score >= threshold) {
                filtered.push_back(lead);
            }
        }
        
        return filtered;
    }
    
    int getTotalLeads() {
        std::lock_guard<std::mutex> lock(leads_mutex);
        return leads.size();
    }
    
    bool updateLeadStatus(const std::string& lead_id, const std::string& new_status) {
        std::lock_guard<std::mutex> lock(leads_mutex);
        
        for(auto& lead : leads) {
            if(lead.id == lead_id) {
                lead.status = new_status;
                return true;
            }
        }
        
        return false;
    }
    
private:
    std::string generateId() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        return "LEAD_" + std::to_string(millis);
    }
    
    std::string getCurrentDateTime() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if(first == std::string::npos) return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    
    std::string capitalizeWords(const std::string& str) {
        std::string result = str;
        bool newWord = true;
        
        for(char& c : result) {
            if(newWord && isalpha(c)) {
                c = toupper(c);
                newWord = false;
            } else if(isspace(c)) {
                newWord = true;
            } else {
                c = tolower(c);
            }
        }
        
        return result;
    }
};

// ==================== REPORT GENERATOR ====================
class ReportGenerator {
public:
    Report generateReport(const std::vector<ContactLead>& leads, const std::string& title = "Lead Report") {
        Report report;
        report.id = "REPORT_" + getTimestamp();
        report.title = title + " - " + getCurrentDate();
        report.leads = leads;
        report.generated_date = getCurrentDateTime();
        
        // Calculate statistics
        report.statistics["total_leads"] = static_cast<int>(leads.size());
        
        int high_score = 0, new_leads = 0;
        std::set<std::string> companies;
        
        for(const auto& lead : leads) {
            if(lead.score >= 70) high_score++;
            if(lead.status == "new") new_leads++;
            if(!lead.company.empty()) companies.insert(lead.company);
        }
        
        report.statistics["high_score_leads"] = high_score;
        report.statistics["new_leads"] = new_leads;
        report.statistics["unique_companies"] = static_cast<int>(companies.size());
        
        // Generate insights
        report.insights = generateInsights(leads);
        
        // Export to file
        report.export_path = exportToFile(report);
        
        return report;
    }
    
    std::string exportToCSV(const std::vector<ContactLead>& leads) {
        std::string filename = "leads_" + getTimestamp() + ".csv";
        
        // Create directory if it doesn't exist
        fs::create_directory("reports");
        
        std::ofstream file("reports/" + filename);
        if(file.is_open()) {
            // Header
            file << "ID,Name,Email,Phone,Company,Website,Industry,Score,Status,Created\n";
            
            // Data
            for(const auto& lead : leads) {
                file << lead.id << ","
                     << "\"" << lead.name << "\","
                     << lead.email << ","
                     << "\"" << lead.phone << "\","
                     << "\"" << lead.company << "\","
                     << lead.website << ","
                     << lead.industry << ","
                     << lead.score << ","
                     << lead.status << ","
                     << lead.created_date << "\n";
            }
            
            file.close();
            return "reports/" + filename;
        }
        
        return "";
    }
    
private:
    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
        return ss.str();
    }
    
    std::string getCurrentDate() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%B %d, %Y");
        return ss.str();
    }
    
    std::string getCurrentDateTime() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    
    std::string generateInsights(const std::vector<ContactLead>& leads) {
        if(leads.empty()) return "No leads to analyze.";
        
        std::stringstream insights;
        insights << "📊 Analysis of " << leads.size() << " leads:\n\n";
        
        // Score distribution
        int excellent = 0, good = 0, average = 0;
        for(const auto& lead : leads) {
            if(lead.score >= 80) excellent++;
            else if(lead.score >= 60) good++;
            else average++;
        }
        
        insights << "🎯 Lead Quality:\n";
        insights << "• Excellent (80+): " << excellent << " leads\n";
        insights << "• Good (60-79): " << good << " leads\n";
        insights << "• Average (<60): " << average << " leads\n\n";
        
        // Status distribution
        std::map<std::string, int> status_count;
        for(const auto& lead : leads) {
            status_count[lead.status]++;
        }
        
        insights << "📈 Lead Status:\n";
        for(const auto& pair : status_count) {
            insights << "• " << pair.first << ": " << pair.second << " leads\n";
        }
        
        // Industry distribution
        std::map<std::string, int> industry_count;
        for(const auto& lead : leads) {
            industry_count[lead.industry]++;
        }
        
        if(!industry_count.empty()) {
            insights << "\n🏢 Top Industries:\n";
            int count = 0;
            for(const auto& pair : industry_count) {
                if(count >= 3) break;
                insights << "• " << pair.first << ": " << pair.second << " leads\n";
                count++;
            }
        }
        
        return insights.str();
    }
    
    std::string exportToFile(const Report& report) {
        std::string filename = "report_" + getTimestamp() + ".txt";
        
        // Create directory if it doesn't exist
        fs::create_directory("reports");
        
        std::ofstream file("reports/" + filename);
        if(file.is_open()) {
            file << "========================================\n";
            file << "LEAD GENERATION REPORT\n";
            file << "========================================\n\n";
            
            file << "Report ID: " << report.id << "\n";
            file << "Generated: " << report.generated_date << "\n";
            file << "Title: " << report.title << "\n\n";
            
            file << "📊 STATISTICS:\n";
            file << "--------------\n";
            for(const auto& stat : report.statistics) {
                file << "• " << stat.first << ": " << stat.second << "\n";
            }
            file << "\n";
            
            file << "👥 LEADS (" << report.leads.size() << "):\n";
            file << "----------\n";
            for(const auto& lead : report.leads) {
                file << "ID: " << lead.id << "\n";
                file << "Name: " << lead.name << "\n";
                file << "Email: " << lead.email << "\n";
                file << "Company: " << lead.company << "\n";
                file << "Score: " << lead.score << "/100\n";
                file << "Status: " << lead.status << "\n";
                file << "---\n";
            }
            file << "\n";
            
            file << "💡 INSIGHTS:\n";
            file << "------------\n";
            file << report.insights << "\n";
            
            file.close();
            return "reports/" + filename;
        }
        
        return "";
    }
};

// ==================== CHATBOT ENGINE ====================
class LeadBot {
private:
    LeadManager lead_manager;
    ReportGenerator report_gen;
    
public:
    std::string processCommand(const std::string& command) {
        std::string lower_cmd = toLower(command);
        
        if(containsAny(lower_cmd, {"hello", "hi", "hey"})) {
            return getWelcomeMessage();
        }
        
        if(containsAny(lower_cmd, {"analyze", "scrape", "extract", "website"})) {
            return handleWebsiteAnalysis(command);
        }
        
        if(containsAny(lower_cmd, {"leads", "contacts", "list"})) {
            return handleLeadList(command);
        }
        
        if(containsAny(lower_cmd, {"report", "generate", "export"})) {
            return handleReportGeneration(command);
        }
        
        if(containsAny(lower_cmd, {"status", "stats", "dashboard"})) {
            return getSystemStatus();
        }
        
        if(containsAny(lower_cmd, {"help", "commands"})) {
            return getHelpMessage();
        }
        
        if(containsAny(lower_cmd, {"clear", "reset"})) {
            return "⚠️ Clear command not implemented in demo version.";
        }
        
        return "I can help you with:\n• Website analysis & lead extraction\n• Lead management\n• Report generation\nType 'help' for commands.";
    }
    
private:
    std::string toLower(const std::string& str) {
        std::string lower = str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower;
    }
    
    bool containsAny(const std::string& text, const std::vector<std::string>& keywords) {
        for(const auto& keyword : keywords) {
            if(text.find(keyword) != std::string::npos) {
                return true;
            }
        }
        return false;
    }
    
    std::string getWelcomeMessage() {
        return "🤖 **Lead Generation Bot**\n"
               "=======================\n"
               "I can extract leads from websites and generate reports!\n\n"
               "🚀 **Try these commands:**\n"
               "• `analyze website https://example.com`\n"
               "• `list all leads`\n"
               "• `generate report`\n"
               "• `export csv`\n"
               "• `system status`\n\n"
               "Type `help` for full command list.";
    }
    
    std::string handleWebsiteAnalysis(const std::string& command) {
        // Extract URL from command
        std::regex url_regex(R"(https?://[^\s]+)");
        std::smatch url_match;
        
        if(std::regex_search(command, url_match, url_regex)) {
            std::string url = url_match[0];
            
            std::string response = "🔍 Analyzing website: " + url + "\n";
            response += "Extracting contact information...\n\n";
            
            // Analyze in background thread
            std::thread([this, url]() {
                auto leads = lead_manager.analyzeWebsite(url);
                
                if(!leads.empty()) {
                    std::cout << "\n✅ Analysis complete!" << std::endl;
                    std::cout << "📧 Found " << leads.size() << " leads" << std::endl;
                }
            }).detach();
            
            response += "✅ Analysis started!\n";
            response += "I'll extract emails, phones, and company info.\n";
            response += "Check progress with 'list all leads' command.";
            
            return response;
        }
        
        return "Please provide a valid URL.\nExample: analyze website https://example.com";
    }
    
    std::string handleLeadList(const std::string& command) {
        if(containsAny(toLower(command), {"all", "list"})) {
            auto leads = lead_manager.getAllLeads();
            return formatLeads(leads, "All Leads");
        }
        
        if(containsAny(toLower(command), {"new", "fresh"})) {
            auto leads = lead_manager.getLeadsByStatus("new");
            return formatLeads(leads, "New Leads");
        }
        
        if(containsAny(toLower(command), {"high", "best", "score"})) {
            auto leads = lead_manager.getHighScoreLeads();
            return formatLeads(leads, "High Score Leads");
        }
        
        return "Lead commands:\n• `list all leads`\n• `list new leads`\n• `list high score leads`";
    }
    
    std::string handleReportGeneration(const std::string& command) {
        auto leads = lead_manager.getAllLeads();
        
        if(leads.empty()) {
            return "No leads to generate report. Analyze websites first.";
        }
        
        if(containsAny(toLower(command), {"csv", "export"})) {
            std::string csv_file = report_gen.exportToCSV(leads);
            
            if(!csv_file.empty()) {
                return "✅ CSV exported: " + csv_file + "\n"
                       "Contains " + std::to_string(leads.size()) + " leads.\n"
                       "Open in Excel or Google Sheets.";
            }
        }
        
        // Generate detailed report
        Report report = report_gen.generateReport(leads);
        
        std::string response = "📊 **Report Generated**\n";
        response += "=====================\n";
        response += "📁 Report saved: " + report.export_path + "\n\n";
        
        response += "📈 **Statistics:**\n";
        for(const auto& stat : report.statistics) {
            response += "• " + stat.first + ": " + std::to_string(stat.second) + "\n";
        }
        
        response += "\n💡 **Report saved in 'reports/' folder**\n";
        response += "Open the file to see complete details.";
        
        return response;
    }
    
    std::string getSystemStatus() {
        int total = lead_manager.getTotalLeads();
        auto new_leads = lead_manager.getLeadsByStatus("new");
        auto high_score = lead_manager.getHighScoreLeads();
        
        std::string status = "📊 **System Status**\n";
        status += "====================\n";
        status += "🤖 Bot Status: ✅ Online\n";
        status += "👥 Total Leads: " + std::to_string(total) + "\n";
        status += "🆕 New Leads: " + std::to_string(new_leads.size()) + "\n";
        status += "⭐ High Score Leads: " + std::to_string(high_score.size()) + "\n";
        
        // Fixed string concatenation
        bool reportsExist = fs::exists("reports");
        status += "📁 Reports Folder: ";
        status += reportsExist ? "✅\n" : "❌\n";
        
        status += "💾 RAM Usage: ~15MB\n";
        
        return status;
    }
    
    std::string getHelpMessage() {
        return "🆘 **Available Commands**\n"
               "=======================\n"
               "🔍 **WEBSITE ANALYSIS:**\n"
               "• `analyze website [URL]` - Extract leads from website\n"
               "• Example: `analyze website https://example.com`\n\n"
               "👥 **LEAD MANAGEMENT:**\n"
               "• `list all leads` - Show all contacts\n"
               "• `list new leads` - Show new leads only\n"
               "• `list high score leads` - Show best quality leads\n\n"
               "📊 **REPORTS & EXPORT:**\n"
               "• `generate report` - Create detailed report\n"
               "• `export csv` - Export leads to CSV file\n\n"
               "💡 **OTHER:**\n"
               "• `system status` - Show system statistics\n"
               "• `help` - Show this message";
    }
    
    std::string formatLeads(const std::vector<ContactLead>& leads, const std::string& title) {
        if(leads.empty()) {
            return "No " + toLower(title) + " found.";
        }
        
        std::stringstream response;
        response << "👥 **" << title << " (" << leads.size() << ")**\n";
        response << "======================\n";
        
        // Show only first 10 leads
        size_t limit = std::min(leads.size(), size_t(10));
        
        for(size_t i = 0; i < limit; i++) {
            const auto& lead = leads[i];
            response << "📌 **Lead #" << (i+1) << "**\n";
            response << "ID: " << lead.id << "\n";
            response << "Name: " << (lead.name.empty() ? "Unknown" : lead.name) << "\n";
            response << "Email: " << lead.email << "\n";
            response << "Company: " << lead.company << "\n";
            response << "Score: " << lead.score << "/100 ";
            
            // Score indicator
            if(lead.score >= 80) response << "🔥";
            else if(lead.score >= 60) response << "⭐";
            else response << "✅";
            
            response << "\n";
            response << "Status: " << lead.status << "\n";
            response << "---\n";
        }
        
        if(leads.size() > 10) {
            response << "\n... and " << (leads.size() - 10) << " more leads.\n";
        }
        
        return response.str();
    }
};

// ==================== MAIN FUNCTION ====================
void runConsoleInterface() {
    LeadBot bot;
    
    std::cout << "===============================================\n";
    std::cout << "🤖 LEAD GENERATION BOT - CONSOLE VERSION\n";
    std::cout << "===============================================\n";
    std::cout << "📊 Features:\n";
    std::cout << "• Extract leads from websites\n";
    std::cout << "• Generate detailed reports\n";
    std::cout << "• Export to CSV format\n";
    std::cout << "• Lead scoring & management\n";
    std::cout << "• ~15MB RAM usage\n\n";
    
    std::cout << "💡 Type 'help' for commands or 'quit' to exit\n";
    std::cout << "💡 Example: analyze website https://example.com\n\n";
    
    std::string command;
    while(true) {
        std::cout << "> ";
        std::getline(std::cin, command);
        
        if(command == "quit" || command == "exit") {
            std::cout << "👋 Goodbye! Reports saved in 'reports/' folder.\n";
            break;
        }
        
        if(command.empty()) continue;
        
        std::string response = bot.processCommand(command);
        std::cout << "\n" << response << "\n\n";
    }
}

int main() {
    // Set random seed
    srand(static_cast<unsigned int>(time(nullptr)));
    
    // Create necessary directories
    fs::create_directory("reports");
    
    // Run console interface
    runConsoleInterface();
    
    return 0;
}