
// FSThrof things
open FSThrof

[<EntryPoint>]
let main argv =
    let tokens = Tokenizer.tokenize(argv.[0])
    let parsedContent = Parser.parse tokens
    let interpretedState = Interpreter.interpret { SymbolTable = parsedContent.SymbolTable; ExecutionStream = parsedContent.ExecutionStream; Stack = [] }
    0
