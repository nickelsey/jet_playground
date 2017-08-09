// Nick Elsey
// 06 - 18 - 17

/*  A set of functions and variables that are used 
    throughout the jetfinding analysis for message
    formatting, error handling, etc. Also, some 
    useful physical constants may be added.
 */


#ifndef JETFINDING_BASE_HH
#define JETFINDING_BASE_HH

/** These are used to format error and status messages to show
    where the output is coming from for easier debugging
 */

#define __ERR(message) {std::cerr << "[" << __FILE__ << "::" << __func__ << "()] -- ERR: " << message << std::endl;}
#define __OUT(message) {std::cout << "[" << __FILE__ << "::" << __func__ << "()] -- OUT: " << message << std::endl;}

/** I took this value from the internet, that could
    be dangerous...
 */

const double pi = 3.141592653589793238462643383279502884;

#endif // JETFINDING_BASE_HH
