#!/usr/local/bin/vipsi

var Start, Duration


proc NewNode(Left,Right,Item)
{
	proc Check() { return /* exists this.Left ? */ Left.Check()-Right.Check()+Item /* : Item */ } 
	return locals
}

proc NewLeaf(Item)
{
	proc Check() { return /* exists this.Left ? Left.Check()-Right.Check()+Item : */ Item } 
	return locals
}

proc BottomUpTree ( item, depth )
{
	if depth
        return NewNode( BottomUpTree(item*2-1, depth-1), BottomUpTree(item*2, depth-1), item )
    else 
        return NewLeaf( item )
	then
}


proc Doit(N)
{
	var minDepth	 = 4
	var maxDepth	 = max( N, minDepth+2 )
    var stretchDepth = maxDepth + 1

    var stretchTree = BottomUpTree( 0, stretchDepth )
	put nl, "stretch tree of depth ", stretchDepth, "\t  check: ", stretchTree.Check()
    del stretchTree

    var longLivedTree = BottomUpTree(0, maxDepth)

	var depth = minDepth -2
	do
		depth += 2
		while depth <= maxDepth 

        var iterations = 1 << (maxDepth - depth + minDepth)
        var check = 0

		var i=0
		do
			while ++i <= iterations 
//			check += BottomUpTree(i, depth).Check()		//  <-- geht nicht wg. <list>.Check 
//			check += BottomUpTree(-i, depth).Check()	//		=> <list> wird gelöst und gelöscht 
//														//		=> Check() findet this nicht mehr
			var tt = BottomUpTree(i,  depth); check += tt.Check()
			var tt = BottomUpTree(-i, depth); check += tt.Check()
		loop

		put nl, iterations*2, "\t  trees of depth ", depth, "\t  check: ", check 
	loop

	put nl, "long lived tree of depth ", maxDepth, "\t  check: ", longLivedTree.Check()
}
		

Start = now		
Doit(4)
Duration = now-Start
put "  --  Total time = ", Duration, " sec."

/*
Start = now		
Doit(10)
Duration = now-Start
put "  --  Total time = ", Duration, " sec."

Start = now		
Doit(12)
Duration = now-Start
put "  --  Total time = ", Duration, " sec."

Start = now		
Doit(14)
Duration = now-Start
put "  --  Total time = ", Duration, " sec."
*/

Start = now		
Doit(16)
Duration = now-Start
put "  --  Total time = ", Duration, " sec."

put nl
end 0
		
		
		
		
		
		
		
		
		
		