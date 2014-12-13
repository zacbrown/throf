namespace FSThrof

module Utility =

    let dropUntil endPattern lst =
        let rec dropUntilHelper = function
        | [] -> []
        | (x :: xs) ->
            if endPattern = x then xs
            else dropUntilHelper xs
        dropUntilHelper lst

    let private collectUntil endPattern lst =
        let rec collectUntilHelper lst acc =
            match lst with
            | [] -> ([], acc)
            | (x :: xs) ->
                if endPattern = x then (xs, List.rev (x :: acc))
                else collectUntilHelper xs (x :: acc)
        collectUntilHelper lst []

