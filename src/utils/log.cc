#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "log.hh"
#include "utils.hh"

namespace genesis {
namespace utils {

// TODO use different init for log details depending on DEBUG
// init static members
LogDetails Log::details = {
    false, // count
    false, // date
    false, // time
    false, // runtime
    false, // rundiff
    false, // file
    false, // line
    true  // level
};
Log::LogLevel              Log::max_level_  = kDebug4;
long                       Log::count_      = 0;
clock_t                    Log::last_clock_ = 0;
std::vector<std::ostream*> Log::ostreams_;

/**
 * Set the highest log level that is reported.
 *
 * Invocations of log with higher levels will create no output.
 * It creates a warning if the set level is higher than the static compile time
 * level set by #LOG_LEVEL_MAX.
 */
void Log::max_level (const LogLevel level)
{
    if (level > LOG_LEVEL_MAX) {
        LOG_WARN << "Log max level set to " << level << ", but compile time "
                 << "max level is " << LOG_LEVEL_MAX << ", so that "
                 << "everything above that will not be logged.";
    }
    max_level_ = level;
}

/**
 * Return a string representation of a log level.
 */
std::string Log::LevelToString(const LogLevel level)
{
    static const char* const buffer[] = {
        "NONE", "ERR ", "WARN", "INFO", "DBG ", "DBG1", "DBG2", "DBG3", "DBG4"
    };
    return buffer[level];
}

/**
 * Add an output stream to which log messages are written.
 */
void Log::AddOutputStream (std::ostream& os)
{
    ostreams_.push_back (&os);
}

/**
 * Add an output file to which log messages are written.
 *
 * This creates a stream to the file.
 */
void Log::AddOutputFile (const std::string fn)
{
    // TODO the log file stream is never deleted. this is not a big leak,
    // as commonly only one file is used for logging, but still is a smell.
    std::ofstream* file = new std::ofstream();
    file->open (fn, std::ios::out | std::ios::app);
    if (file->is_open()) {
        ostreams_.push_back (file);
    } else {
        LOG_WARN << "Cannot open log file " << fn;
    }
}

/**
 * Constructor, does nothing.
 */
Log::Log()
{
}

// TODO the output of the log is not thread safe
// TODO use auto indention for multi line log messages
// TODO strip more than one endl at the end
/**
 * Destructor that is invoked at the end of each log line and does the actual
 * output.
 */
Log::~Log()
{
    // build the details for the log message into a buffer
    clock_t now_clock = clock();
    std::ostringstream det_buff;
    det_buff.str("");
    if (details_.count) {
        det_buff.fill('0');
        det_buff.width(4);
        det_buff << count_ << " ";
    }
    if (details_.date) {
        det_buff << CurrentDate() << " ";
    }
    if (details_.time) {
        det_buff << CurrentTime() << " ";
    }
    if (details_.runtime) {
        det_buff << std::fixed
                 << std::setprecision(6)
                 << double(now_clock) / CLOCKS_PER_SEC
                 << " ";
    }
    if (details_.rundiff) {
        double val = 0.0;
        if (last_clock_ > 0) {
            val = (double) (now_clock - last_clock_) / CLOCKS_PER_SEC;
        }
        det_buff << std::fixed
                 << std::setprecision(6)
                 << val
                 << " ";
        last_clock_ = now_clock;
    }
    if (details_.file) {
        det_buff << file_ << (details_.line ? "" : " ");
    }
    if (details_.line) {
        det_buff << ":" << line_ << " ";
    }
    if (details_.level) {
        det_buff << LevelToString(level_) << " ";
    }

    // add spaces for nested debug levels
    if (level_ > kDebug) {
        for (int i = 0; i < level_ - kDebug; i++) {
            det_buff << "  ";
        }
    }

    // output the message to every stream
    for (std::ostream* out : ostreams_) {
        (*out) << det_buff.str() << buff_.str() << std::endl << std::flush;
    }

    // inc log message counter
    count_++;
}

/**
 * Getter for the singleton instance of log, is called by the standard macros.
 *
 * It returns the string stream buffer used to capture the log messages.
 */
std::ostringstream& Log::Get(
    const std::string file, const int line, const LogLevel level
)
{
    return Get(file, line, level, details);
}

/**
 * Getter for the singleton instance of log, is called by special macros
 * that change the details of the log message.
 *
 * It stores some relevant information and returns the string stream buffer
 * used to capture the log messages.
 */
std::ostringstream& Log::Get(
    const std::string file, const int line,
    const LogLevel level, const LogDetails dets
)
{
    // save the information given when called from the macros
    file_    = file;
    line_    = line;
    level_   = level;
    details_ = dets;
    buff_.str("");
    return buff_;
}

} // namespace utils
} // namespace genesis
