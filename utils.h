#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <sstream>
#include <string>



// Codes below are copied from open_spiel.
namespace internal {
// SpielStrOut(out, a, b, c) is equivalent to:
//    out << a << b << c;
// It is used to enable SpielStrCat, below.
template <typename Out, typename T>
void SpielStrOut(Out& out, const T& arg) {
  out << arg;
}

template <typename Out, typename T, typename... Args>
void SpielStrOut(Out& out, const T& arg1, Args&&... args) {
  out << arg1;
  SpielStrOut(out, std::forward<Args>(args)...);
}

// Builds a string from pieces:
//
//  SpielStrCat(1, " + ", 1, " = ", 2) --> "1 + 1 = 2"
//
// Converting the parameters to strings is done using the stream operator<<.
// This is only kept around to be used in the SPIEL_CHECK_* macros and should
// not be called by any code outside of this file. Prefer absl::StrCat instead.
// It is kept here due to support for more types, including char.
template <typename... Args>
std::string SpielStrCat(Args&&... args) {
  std::ostringstream out;
  SpielStrOut(out, std::forward<Args>(args)...);
  return out.str();
}

}  // namespace internal

void SpielDefaultErrorHandler(const std::string& error_msg) {
  std::cerr << "Spiel Fatal Error: " << error_msg << std::endl
            << std::endl
            << std::flush;
  std::exit(1);
}

// Specify a new error handler.
using ErrorHandler = void (*)(const std::string&);

ErrorHandler error_handler = SpielDefaultErrorHandler;

void SetErrorHandler(ErrorHandler new_error_handler) {
  error_handler = new_error_handler;
}

// Report a runtime error.
[[noreturn]] void SpielFatalError(const std::string& error_msg) {
  error_handler(error_msg);
  // The error handler should not return. If it does, we will abort the process.
  std::cerr << "Error handler failure - exiting" << std::endl;
  std::exit(1);
}

// Macros to check for error conditions.
// These trigger SpielFatalError if the condition is violated.
// These macros are always executed. If you want to use checks
// only for debugging, use SPIEL_DCHECK_*

#define SPIEL_CHECK_OP(x_exp, op, y_exp)                           \
  do {                                                             \
    auto x = x_exp;                                                \
    auto y = y_exp;                                                \
    if (!((x)op(y)))                                               \
      SpielFatalError(internal::SpielStrCat(                       \
          __FILE__, ":", __LINE__, " ", #x_exp " " #op " " #y_exp, \
          "\n" #x_exp, " = ", x, ", " #y_exp " = ", y));           \
  } while (false)

#define SPIEL_CHECK_FN2(x_exp, y_exp, fn)                                 \
  do {                                                                    \
    auto x = x_exp;                                                       \
    auto y = y_exp;                                                       \
    if (!fn(x, y))                                                        \
      SpielFatalError(internal::SpielStrCat(                              \
          __FILE__, ":", __LINE__, " ", #fn "(" #x_exp ", " #y_exp ")\n", \
          #x_exp " = ", x, ", " #y_exp " = ", y));                        \
  } while (false)

#define SPIEL_CHECK_FN3(x_exp, y_exp, z_exp, fn)                         \
  do {                                                                   \
    auto x = x_exp;                                                      \
    auto y = y_exp;                                                      \
    auto z = z_exp;                                                      \
    if (!fn(x, y, z))                                                    \
      SpielFatalError(internal::SpielStrCat(                             \
          __FILE__, ":", __LINE__, " ",                                  \
          #fn "(" #x_exp ", " #y_exp ", " #z_exp ")\n", #x_exp " = ", x, \
          ", " #y_exp " = ", y, ", " #z_exp " = ", z));                  \
  } while (false)

#define SPIEL_CHECK_GE(x, y) SPIEL_CHECK_OP(x, >=, y)
#define SPIEL_CHECK_GT(x, y) SPIEL_CHECK_OP(x, >, y)
#define SPIEL_CHECK_LE(x, y) SPIEL_CHECK_OP(x, <=, y)
#define SPIEL_CHECK_LT(x, y) SPIEL_CHECK_OP(x, <, y)
#define SPIEL_CHECK_EQ(x, y) SPIEL_CHECK_OP(x, ==, y)
#define SPIEL_CHECK_NE(x, y) SPIEL_CHECK_OP(x, !=, y)
#define SPIEL_CHECK_PROB(x) \
  SPIEL_CHECK_GE(x, 0);     \
  SPIEL_CHECK_LE(x, 1);     \
  SPIEL_CHECK_FALSE(std::isnan(x) || std::isinf(x))
#define SPIEL_CHECK_PROB_TOLERANCE(x, tol) \
  SPIEL_CHECK_GE(x, -(tol));               \
  SPIEL_CHECK_LE(x, 1.0 + (tol));          \
  SPIEL_CHECK_FALSE(std::isnan(x) || std::isinf(x))

#define SPIEL_CHECK_TRUE(x) \
  while (!(x))              \
  SpielFatalError(          \
      internal::SpielStrCat(__FILE__, ":", __LINE__, " CHECK_TRUE(", #x, ")"))

#define SPIEL_CHECK_FALSE(x)                                     \
  while (x)                                                      \
  SpielFatalError(internal::SpielStrCat(__FILE__, ":", __LINE__, \
                                        " CHECK_FALSE(", #x, ")"))

#if !defined(NDEBUG)

// Checks that are executed in Debug / Testing build type,
// and turned off for Release build type.
#define SPIEL_DCHECK_OP(x_exp, op, y_exp) SPIEL_CHECK_OP(x_exp, op, y_exp)
#define SPIEL_DCHECK_FN2(x_exp, y_exp, fn) SPIEL_CHECK_FN2(x_exp, y_exp, fn)
#define SPIEL_DCHECK_FN3(x_exp, y_exp, z_exp, fn) \
  SPIEL_CHECK_FN3(x_exp, y_exp, z_exp, fn)
#define SPIEL_DCHECK_GE(x, y) SPIEL_CHECK_GE(x, y)
#define SPIEL_DCHECK_GT(x, y) SPIEL_CHECK_GT(x, y)
#define SPIEL_DCHECK_LE(x, y) SPIEL_CHECK_LE(x, y)
#define SPIEL_DCHECK_LT(x, y) SPIEL_CHECK_LT(x, y)
#define SPIEL_DCHECK_EQ(x, y) SPIEL_CHECK_EQ(x, y)
#define SPIEL_DCHECK_NE(x, y) SPIEL_CHECK_NE(x, y)
#define SPIEL_DCHECK_PROB(x) SPIEL_DCHECK_PROB(x)
#define SPIEL_DCHECK_FLOAT_EQ(x, y) SPIEL_CHECK_FLOAT_EQ(x, y)
#define SPIEL_DCHECK_FLOAT_NEAR(x, y, epsilon) \
  SPIEL_CHECK_FLOAT_NEAR(x, y, epsilon)
#define SPIEL_DCHECK_TRUE(x) SPIEL_CHECK_TRUE(x)
#define SPIEL_DCHECK_FALSE(x) SPIEL_CHECK_FALSE(x)

#else  // defined(NDEBUG)

// Turn off checks for the (optimized) Release build type.
#define SPIEL_DCHECK_OP(x_exp, op, y_exp)
#define SPIEL_DCHECK_FN2(x_exp, y_exp, fn)
#define SPIEL_DCHECK_FN3(x_exp, y_exp, z_exp, fn)
#define SPIEL_DCHECK_GE(x, y)
#define SPIEL_DCHECK_GT(x, y)
#define SPIEL_DCHECK_LE(x, y)
#define SPIEL_DCHECK_LT(x, y)
#define SPIEL_DCHECK_EQ(x, y)
#define SPIEL_DCHECK_NE(x, y)
#define SPIEL_DCHECK_PROB(x)
#define SPIEL_DCHECK_FLOAT_EQ(x, y)
#define SPIEL_DCHECK_FLOAT_NEAR(x, y, epsilon)
#define SPIEL_DCHECK_TRUE(x)
#define SPIEL_DCHECK_FALSE(x)

#endif  // !defined(NDEBUG)

#endif /* UTILS_H_ */
