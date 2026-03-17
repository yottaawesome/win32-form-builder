export module formbuilder:errors;
import std;

export namespace FormDesigner
{
	enum class FormErrorCode
	{
		ParseError,              // Malformed JSON syntax
		InvalidField,            // Wrong type or value for a JSON field
		UnknownControlType,      // Unrecognized control type string
		FileNotFound,            // File doesn't exist or can't be opened for reading
		FileWriteError,          // Can't open or write to file
		WindowCreationFailed,    // CreateWindowExW returned nullptr
		ClassRegistrationFailed, // RegisterClassExW failed
	};

	class FormException : public std::runtime_error
	{
	public:
		FormException(FormErrorCode code, const std::string& message)
			: std::runtime_error(message), code_(code) {}

		auto code() const noexcept -> FormErrorCode { return code_; }

	private:
		FormErrorCode code_;
	};
}
