
// FSThrof things
open FSThrof

[<EntryPoint>]
let main argv =
    let foo = Interpreter.loadFile <| { SymbolTable = Map.empty; ExecutionStream = []; Stack = [] } <| argv.[0]
    0
