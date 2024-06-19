#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

/**
 * @brief Structure representing a variable with optional dependency-based values.
 */
struct Variable {
    std::string type;  ///< Type of the variable.
    std::string name;  ///< Name of the variable.
    std::string value; ///< Value of the variable.
    std::optional<std::string> dep; ///< Optional dependency (mode or type).
    std::unordered_map<std::string, std::string> depValues; ///< Values based on dependency.
};

/**
 * @brief Structure representing a namespace.
 */
struct Namespace {
    std::string name; ///< Name of the namespace.
    std::vector<Variable> variables; ///< List of variables in the namespace.
    std::unordered_map<std::string, Namespace> namespaces; ///< Nested namespaces.
};

/**
 * @brief Class for generating C++ header files from HJSON configuration.
 */
class HeaderGenerator {
public:
    /**
     * @brief Constructs a HeaderGenerator.
     * @param copyright Copyright notice for the generated header file.
     * @param filename Name of the output header file.
     */
    HeaderGenerator(const std::string& copyright, const std::string& filename);

    /**
     * @brief Adds a namespace to the generator.
     * @param name Name of the namespace to add.
     */
    void addNamespace(const std::string& name);

    /**
     * @brief Adds a variable to a namespace.
     * @param namespaceName Name of the namespace.
     * @param type Type of the variable.
     * @param name Name of the variable.
     * @param value Value of the variable.
     * @param dep Optional dependency (mode or type).
     * @param depValues Map of values based on the dependency.
     */
    void addVariable(const std::string& namespaceName, const std::string& type, const std::string& name, const std::string& value, const std::optional<std::string>& dep = std::nullopt, const std::unordered_map<std::string, std::string>& depValues = {});

    /**
     * @brief Generates the header file.
     */
    void generate() const;

    /**
     * @brief Parses an HJSON file to configure namespaces and variables.
     * @param hjsonFilename Path to the HJSON file.
     */
    void parseHjson(const std::string& hjsonFilename);

private:
    std::string filename; ///< Name of the output header file.
    std::string copyright; ///< Copyright notice.
    Namespace rootNamespace; ///< Root namespace.
    std::string projectMode; ///< Project mode (development or production).
    std::string projectType; ///< Project type (debug or release).

    /**
     * @brief Generates code for a namespace and its contents.
     * @param out Output stream.
     * @param ns Namespace to generate.
     * @param indent Indentation string.
     */
    void generateNamespace(fmt::ostream& out, const Namespace& ns, const std::string& indent) const;

    /**
     * @brief Processes an HJSON object into a namespace.
     * @param currentNamespace Current namespace being processed.
     * @param obj HJSON object.
     */
    void processHjsonObject(Namespace& currentNamespace, const Hjson::Value& obj);

    /**
     * @brief Applies dependency-based configurations to variables.
     */
    void applyDepConfig();

    /**
     * @brief Applies dependency-based configurations to variables within a namespace.
     * @param ns Namespace to process.
     */
    void applyDepConfigToNamespace(Namespace& ns);

    /**
     * @brief Resolves the value of a variable based on its dependency.
     * @param var Variable to resolve.
     * @return Resolved value.
     */
    std::string resolveDepValue(const Variable& var) const;
};
