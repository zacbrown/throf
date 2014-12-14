
// FSThrof things
open FSThrof

[<EntryPoint>]
let main argv =
    if System.IO.File.Exists("init.th4") then
        let initCoreState = Interpreter.loadFile <| { SymbolTable = Map.empty; ExecutionStream = []; Stack = [] } <| "init.th4"
        Interpreter.loadFile <| initCoreState <| argv.[0] |> ignore
    else
        Interpreter.loadFile <| { SymbolTable = Map.empty; ExecutionStream = []; Stack = [] } <| argv.[0] |> ignore
    0
