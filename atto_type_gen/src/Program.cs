using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.NetworkInformation;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace SirReflector
{

    public class Program
    {
        public static string BASE_PATH = "C://Projects/Atto - G4/atto/";

        public static string[] ReadAllHeaderFiles(string path) {
            string[] fileNames = Directory.GetFiles(path, "*.h", SearchOption.AllDirectories);
            string[] files = new string[fileNames.Length];
            for (int i = 0; i < fileNames.Length; i++) {
                //Console.WriteLine("Reading: " + fileNames[i]);
                files[i] = File.ReadAllText(fileNames[i]);
            }
            return files;
        }

        public class CppVariable
        {
            public string type;
            public string name;
        }

        public class StructDef
        {
            public string name = "";
            public List<CppVariable> variables = new List<CppVariable>();
        }

        public class EnumDef {
            public string name = "";
            public List<string> values = new List<string>();
        }

        public static string RemoveWhitespace(string input) {
            if (input == null) return null;
            return string.Concat(input.Where(c => !Char.IsWhiteSpace(c)));
        }

        public static CppVariable ParseCppDeclaration(string declaration) {
            if (declaration.EndsWith(";")) {
                declaration = declaration.Substring(0, declaration.Length - 1);
            }

            int spaceIndex = declaration.LastIndexOf(' ');

            if (spaceIndex == -1) {
                throw new ArgumentException("Invalid declaration format.");
            }

            string type = RemoveWhitespace(declaration.Substring(0, spaceIndex));
            string name = RemoveWhitespace(declaration.Substring(spaceIndex + 1));

            return new CppVariable {
                type = type,
                name = name
            };
        }

        public static StructDef ParseStructCode(string source) {
            StructDef def = new StructDef();

            StringReader reader = new StringReader(source);
            string defLine = reader.ReadLine();

            defLine = RemoveWhitespace(defLine);
            defLine = defLine.Replace("struct", "");
            defLine = defLine.Replace("class", "");
            defLine = defLine.Replace("{", "");
            def.name = defLine;

            //Console.WriteLine("Name={0}", defLine);

            while (true) {
                string line = reader.ReadLine();
                if (line == null) {
                    break;
                }

                if (line.Contains("};")) {
                    break;
                }

                if (!line.EndsWith(";")) {
                    continue;
                }

                if (line.Contains("(")) { // For functions
                    continue;
                }

                if (line.Contains("*")) {
                    Console.WriteLine("Skipping pointer: {0}", line);
                    continue;
                }

                if (line.Contains("std::")) {
                    Console.WriteLine("Skipping std object: {0}", line);
                    continue;
                }

                if (line.Contains("AF_IGNORE")) {
                    continue;
                }

                CppVariable cppVar = ParseCppDeclaration(line);
                //Console.WriteLine("Type={0} Name={1}", cppVar.type, cppVar.name);
                def.variables.Add(cppVar);
            }

            return def;
        }

        public static EnumDef ParseEnumCode(string source) {
            EnumDef def = new EnumDef();

            StringReader reader = new StringReader(source);
            string defLine = reader.ReadLine();

            defLine = RemoveWhitespace(defLine);
            defLine = defLine.Replace("enum", "");
            defLine = defLine.Replace("class", "");
            defLine = defLine.Replace("{", "");
            def.name = defLine;

            while (true) {
                string line = reader.ReadLine();
                if (line == null) {
                    break;
                }

                if (line.Contains("};")) {
                    break;
                }

                line = RemoveWhitespace(line);
                int index = line.IndexOf('=');
                if (index >= 0) {
                    line = line.Substring(0, index);
                } else {
                    index = line.IndexOf(',');
                    if (index >= 0) {
                        line = line.Substring(0, index);
                    }
                }
                
                def.values.Add(line);
            }

            return def;
        }

        public static string ATTO_REFLECT_STRUCT = "ATTO_REFLECT_STRUCT";
        public static List<StructDef> ParseStructs(string source) {
            StringReader reader = new StringReader(source);
            List<StructDef> defs = new List<StructDef>();

            while (true) {
                string line = reader.ReadLine();
                if (line == null) {
                    break;
                }

                if (line.Contains("#define")) {
                    continue;
                }

                if (line.Contains(ATTO_REFLECT_STRUCT)) {
                    // @TODO: Assumes that the struct def is directly after define
                    StringBuilder builder = new StringBuilder();
                    while (true) {
                        line = reader.ReadLine();
                        builder.AppendLine(line);

                        // @TODO: This is ok for now but will cause issues later
                        if (line.Contains("};")) {
                            break;
                        }
                    }

                    StructDef def = ParseStructCode(builder.ToString());
                    defs.Add(def);
                }
                
            }

            return defs;
        }

        public static string ATTO_REFLECT_ENUM = "ATTO_REFLECT_ENUM";
        public static List<EnumDef> ParseEnums(string source) {
            StringReader reader = new StringReader(source);
            List<EnumDef> defs = new List<EnumDef>();

            while (true) {
                string line = reader.ReadLine();
                if (line == null) {
                    break;
                }

                if (line.Contains("#define")) {
                    continue;
                }

                if (line.Contains(ATTO_REFLECT_ENUM)) {
                    StringBuilder builder = new StringBuilder();
                    while (true) {
                        line = reader.ReadLine();
                        builder.AppendLine(line);

                        // @TODO: This is ok for now but will cause issues later
                        if (line.Contains("};")) {
                            break;
                        }
                    }

                    EnumDef def = ParseEnumCode(builder.ToString());
                    defs.Add(def);
                }

            }

            return defs;
        }
        
        public static void WriteCppGenFile(List<StructDef> structDefs, List<EnumDef> enumDefs) {
            StringBuilder builder = new StringBuilder();
            builder.AppendLine("#include \"AttoReflection.h\"");
            builder.AppendLine("#include \"AttoOperatingSystem.h\"");
            builder.AppendLine("#include \"AttoLogging.h\"");
            builder.AppendLine("#include \"AttoGame.h\"");
            
            builder.AppendLine("namespace atto {");

            foreach (StructDef def in structDefs) {
                builder.AppendLine($"    nlohmann::json JSON_Write(const {def.name}& obj) {"{"}");
                builder.AppendLine("        nlohmann::json j = {};");

                foreach (CppVariable cppVar in def.variables) {
                    builder.AppendLine($"        j[\"{cppVar.name}\"] = JSON_Write(obj.{cppVar.name});");
                }

                builder.AppendLine("        return j;");

                builder.AppendLine("    }");
            }

            foreach (EnumDef def in enumDefs) {
                builder.AppendLine($"    const char * EnumToString({def.name} obj) {"{"}");
                builder.AppendLine("        switch(obj) {");
                foreach (string var in def.values) {
                    builder.AppendLine($"            case {def.name}::{var}: return \"{var}\";");
                }
                builder.AppendLine("        }");
                builder.AppendLine("        return \"UNKNOWN_ENUM_STRING_VALUE\";");
                builder.AppendLine("    }");
            }

            builder.AppendLine("}");

            string genSource = builder.ToString();
            //Console.WriteLine(genSource);

            File.WriteAllText(BASE_PATH + "src/AttoReflection_Gen.cpp", genSource);
        }
        public static void WriteHeaderGenFile(List<StructDef> structDefs) {
            StringBuilder builder = new StringBuilder();
            builder.AppendLine("#pragma once");
            builder.AppendLine("#include \"AttoReflection.h\"");
            builder.AppendLine("namespace atto {");

            foreach (StructDef def in structDefs) {
                builder.AppendLine("    template<>");
                builder.AppendLine($"    {def.name} JSON_Read<{def.name}>(const nlohmann::json &j) {"{"}");
                builder.AppendLine($"        {def.name} obj = {"{}"};");

                foreach (CppVariable cppVar in def.variables) {
                    if (cppVar.type.Contains("FixedList")) {
                        builder.AppendLine($"        obj.{cppVar.name} = JSON_Read_{cppVar.type}(j[\"{cppVar.name}\"]);");
                    }
                    else {
                        builder.AppendLine($"        obj.{cppVar.name} = JSON_Read<{cppVar.type}>(j[\"{cppVar.name}\"]);");
                    }
                }

                builder.AppendLine("        return obj;");

                builder.AppendLine("    }");
            }

            builder.AppendLine("}");

            string genSource = builder.ToString();
            //Console.WriteLine(genSource);

            File.WriteAllText(BASE_PATH + "src/AttoReflection_Gen.h", genSource);
        }

        public static void Main(string[] args) {
            Console.WriteLine("Reflecting...");
            string[] files = ReadAllHeaderFiles(BASE_PATH + "src/");

            List<StructDef> structDefs = new List<StructDef>();
            foreach (string file in files) {
                structDefs = structDefs.Concat(ParseStructs(file)).ToList();
            }

            List<EnumDef> enumDefs = new List<EnumDef>();
            foreach (string file in files) {
                enumDefs = enumDefs.Concat(ParseEnums(file)).ToList();
            }

            WriteHeaderGenFile(structDefs);
            WriteCppGenFile(structDefs, enumDefs);


            Console.WriteLine("Done.");
        }

    }
}
