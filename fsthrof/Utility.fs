namespace FSThrof

module List =
    let skipWhile (pred:'a -> bool) (lst:'a list) =
        let rec skipWhileHelper = function
        | [] -> []
        | (x :: xs) ->
            if pred x then xs
            else skipWhileHelper xs
        skipWhileHelper lst

    let takeWhile (pred:'a -> bool) (lst:'a list) =
        let rec takeWhileHelper lst acc =
            match lst with
            | [] -> ([], acc)
            | (x :: xs) ->
                if pred x then (xs, List.rev (x :: acc))
                else takeWhileHelper xs (x :: acc)
        takeWhileHelper lst []