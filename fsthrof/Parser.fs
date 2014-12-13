namespace FSThrof

open System

module Parser =
    type Directive =
        | LoadFile
        | Defer
        | Variable
    and Node =
        | Directive of directive : Directive
        | Word of data : string
        | Integer of num : int
        | Real of num : double
        | Boolean of value : bool
        | StringLiteral of data : string
        | Quotation of list<Node>
        with
            member this.IsTruthy() =
                match this with
                | Integer 0 | Real 0.0 | Boolean false -> false
                | _ -> true
    and ParsedContent = {
        SymbolTable : Map<string, list<Node>>;
        ExecutionStream : list<Node>;
    }

    exception InvalidTokenToNodeTransformation of string
    exception UnexpectedToken of string
    exception InvalidDirective of string

    let private tokenToNode (tok : Tokenizer.Token) =
        match tok with
        | Tokenizer.Directive directive ->
            match directive with
            | ":include" -> Directive LoadFile
            | ":variable" -> Directive Variable
            | ":defer" -> Directive Defer
            | str -> raise <| InvalidDirective str
        | Tokenizer.WordOrData data ->
            try
                let numVal = Int32.Parse data
                Integer numVal
            with
            | :? System.FormatException as ex ->
                try
                    let numVal = Double.Parse data
                    Real numVal
                with
                | :? System.FormatException as ex ->
                    try
                        let boolVal = Boolean.Parse data
                        Boolean boolVal
                    with
                    | :? System.FormatException as ex ->
                        Word data
        | Tokenizer.StringLiteral data -> StringLiteral data
        | _ -> raise <| InvalidTokenToNodeTransformation (sprintf "%A" tok)

    let private collectQuotation tokens =
        let (streamRemainder, quotationStream) = (List.takeWhile (fun tok -> Tokenizer.QuotationClose = tok) tokens)
        let quotationNode = Quotation (List.map (fun elem -> tokenToNode elem) quotationStream)
        (streamRemainder, quotationNode)

    let rec private transformTokensToNodes acc lst =
        match lst with
        | [] -> List.rev acc
        | ((hd : Tokenizer.Token) :: tl) ->
            match hd with
            | Tokenizer.QuotationOpen ->
                let (remainingStream, quotationNode) = collectQuotation tl
                transformTokensToNodes (quotationNode :: acc) remainingStream
            | Tokenizer.StringLiteral data -> transformTokensToNodes (StringLiteral data :: acc) tl
            | Tokenizer.WordOrData _ ->
                transformTokensToNodes ((tokenToNode hd) :: acc) tl
            | _ -> raise <| UnexpectedToken (sprintf "%A" hd)

    let parse (tokens : list<Tokenizer.Token>) =
        let rec parseHelper tokens (content : ParsedContent) =
            match tokens with
            | [] -> content
            | (tok : Tokenizer.Token) :: toks ->
                match tok with
                | Tokenizer.WordDefinition ->
                    match toks.Head with
                    | Tokenizer.WordOrData wordName ->
                        let (streamRemainder, wordDef) = (List.takeWhile (fun tok -> Tokenizer.DefinitionTerminator = tok) toks)
                        let newContent = {
                            SymbolTable = content.SymbolTable.Add(wordName, transformTokensToNodes [] wordDef);
                            ExecutionStream = content.ExecutionStream
                        }
                        parseHelper streamRemainder newContent
                    | _ -> raise <| UnexpectedToken (sprintf "%+A" toks.Head)
                | Tokenizer.Directive _ ->
                    let parsedDirective = tokenToNode tok
                    match parsedDirective with
                    | Directive LoadFile ->
                        match toks with
                        | (Tokenizer.StringLiteral filename :: streamRemainder) ->
                            let newContent = {
                                SymbolTable = content.SymbolTable;
                                ExecutionStream = (StringLiteral filename :: Directive LoadFile :: content.ExecutionStream)
                            }
                            parseHelper streamRemainder newContent
                        | _ -> raise <| UnexpectedToken (sprintf "%+A" toks.Head)
                    | Directive Defer ->
                        match toks with
                        | (Tokenizer.WordOrData deferredWord :: streamRemainder) ->
                            let newContent = {
                                SymbolTable = content.SymbolTable.Add(deferredWord, []);
                                ExecutionStream = content.ExecutionStream
                            }
                            parseHelper streamRemainder newContent
                        | _ -> raise <| UnexpectedToken (sprintf "%+A" toks.Head)
                    | Directive Variable ->
                        match toks with
                        | (Tokenizer.WordOrData variableName :: value :: streamRemainder) ->
                            let newContent = {
                                SymbolTable = content.SymbolTable.Add(variableName, [tokenToNode value]);
                                ExecutionStream = content.ExecutionStream
                            }
                            parseHelper streamRemainder newContent
                        | _ -> raise <| UnexpectedToken (sprintf "%+A" toks.Head)
                    | _ -> raise <| UnexpectedToken (sprintf "%+A" toks.Head)
                | Tokenizer.QuotationOpen ->
                    let (streamRemainder, quotationNode) = collectQuotation tokens
                    let newContent = {
                        SymbolTable = content.SymbolTable;
                        ExecutionStream = content.ExecutionStream @ [quotationNode]
                    }
                    parseHelper streamRemainder newContent
                | Tokenizer.WordOrData _ ->
                    let newContent = {
                        SymbolTable = content.SymbolTable;
                        ExecutionStream = content.ExecutionStream @ [tokenToNode tok]
                    }
                    parseHelper toks newContent
                | Tokenizer.StringLiteral data ->
                    let newContent = {
                        SymbolTable = content.SymbolTable;
                        ExecutionStream = content.ExecutionStream @ [StringLiteral data]
                    }
                    parseHelper toks newContent
                | _ -> raise <| UnexpectedToken (sprintf "%A" tok)
        parseHelper tokens { SymbolTable = Map.empty; ExecutionStream = []}

    let getParserState (parsedContent : ParsedContent) =
        let symTable = Map.map (fun x y -> sprintf "%+A - %+A" x y) parsedContent.SymbolTable
        let execStream = List.map (fun x -> sprintf "%+A" x) parsedContent.ExecutionStream
        (symTable, execStream)

    let printParserState (parsedContent : ParsedContent) =
        let (symTable, execStream) = getParserState parsedContent
        printfn "Parser State"
        printfn "================="
        Map.iter (fun x -> printfn "%+A - %+A" x) symTable
        List.iter (fun x -> printfn "%+A" x) execStream
        printfn "================="
