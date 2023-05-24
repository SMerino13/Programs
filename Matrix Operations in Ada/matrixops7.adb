with Ada.Directories;   use Ada.Directories;
with Ada.Text_IO;       use Ada.Text_IO;
with Ada.Strings.Fixed; use Ada.Strings.Fixed;
with Ada.Integer_Text_IO; use Ada.Integer_Text_IO;
with Ada.Long_Float_Text_IO; use Ada.Long_Float_Text_IO;


procedure matrixops7 is

	Directory : String := ".";
    Pattern   : String := ""; -- empty pattern = all file subdirectory names
	Search    : Search_Type;
    Dir_Ent   : Directory_Entry_Type;
	smatch    : String := "in";


    ROWS : constant := 7;
    COLS : constant := 7;
    inFile, outFile : FILE_TYPE;

    type MATRIX is array
    (NATURAL range 1..ROWS, NATURAL range 1..COLS) of LONG_FLOAT;

    -- Matrix m1 - m3 HOLDS VALUES READ FROM FILES 
    m1 : MATRIX;
    m2 : MATRIX;
    m3 : MATRIX;
    -- MATRIX tmpMatrix1 - tmpMatrix4 ARE USED AS TEMP ARRAYS FOR OPERATIONS
    tmpMatrix1 : MATRIX;
    tmpMatrix2 : MATRIX;
    tmpMatrix3 : MATRIX;
    tmpMatrix4 : MATRIX;
    -- MATRIX HOLDS THE INVERSE OF A SPECIFIED MATRIX
    inverseMatrix : MATRIX;
    -- VARIABLES
    Row         : NATURAL := 0;
    Column      : NATURAL := 0;
    Scalar      : LONG_FLOAT;
    d           : LONG_FLOAT := 5.0;
    Dimension   : INTEGER := 7; 
    Determin    : LONG_FLOAT := 0.0; 
    inVal       : INTEGER;
    coRow       : INTEGER;
    coCol       : INTEGER;

-- PROCEDURE PRINTS A SPECIFIED MATRIX IN A SPECIFIC FORMAT
    procedure PrintMatrix (Arr : MATRIX) is 
    begin 
    -- PRINT RESULTS OF OPERATION
        for Row in MATRIX'Range(1) loop
            for Column in MATRIX'Range(2) loop
                if(Arr(Row, Column) > 0.9 or Arr(Row, Column) < -0.9) then
                    Ada.Long_Float_Text_IO.Put(Arr(Row, Column), 18, 0, 0);
                elsif(Arr(Row, Column) < 0.09 or Arr(Row, Column) > -0.09) then
                    Ada.Long_Float_Text_IO.Put(Arr(Row, Column), 5, 15, 0);
                end if;
            end loop;
            Put_Line("");
        end loop;
        Put_Line("");
    end PrintMatrix;

-- PROCEDURE ONLY PRINTS MATRIX'S WITH VALUES THAT ARE EXTERMLY SMALL
    procedure InversePrintMatrix (Arr : MATRIX) is 
    begin 
    -- PRINT RESULTS OF OPERATION
        for Row in MATRIX'Range(1) loop
            for Column in MATRIX'Range(2) loop
                Ada.Long_Float_Text_IO.Put(Arr(Row, Column), 5, 8, 3);
            end loop;
            Put_Line("");
        end loop;
        Put_Line("");
    end InversePrintMatrix;

-- MATRIX ADDITION PROCEDURE
    procedure Addition (Arr : in out MATRIX; Arr2: in out MATRIX;
        Result: in out MATRIX) is
    
    begin
        for Row in MATRIX'Range(1) loop
            for Column in MATRIX'Range(2) loop
                Result(Row, Column) := Arr(Row, Column) + Arr2(Row, Column);
            end loop;
        end loop;

    end Addition;

-- MATRIX SUBTRACTION PROCEDURE
    procedure Subtraction (Arr : in out MATRIX; Arr2: in out MATRIX;
        Result: in out MATRIX) is
    begin
        for Row in MATRIX'Range(1) loop
            for Column in MATRIX'Range(2) loop
                Result(Row, Column) := Arr(Row, Column) - Arr2(Row, Column);
            end loop;
        end loop;

    end Subtraction;

-- MATRIX MULTIPLICATION PROCEDURE
    procedure Multiplication (Arr : in out MATRIX; Arr2: in out MATRIX;
        Result: in out MATRIX) is
    -- VARIABLES FOR MULTIPLICATION PRODUCT
    multiProduct : LONG_FLOAT;
    Final : LONG_FLOAT := 0.0;

    begin
        for Row in MATRIX'Range(1) loop
            for Column in MATRIX'Range(2) loop
            
            -- ITERATE OVER ROWS AND COLS AT SAME TIME TO CALC EACH INDEX
                Final := 0.0;
                for i in MATRIX'Range(1) loop
                    multiProduct := Arr(Row, i) * Arr2(i, Column);
                    Final := Final + multiProduct;

                end loop;
                Result(Row, Column) := Final;
            end loop;
        end loop;

    end Multiplication;

-- PROCEDURE MULITPLIES MATRIX BY A SPECIFIED SCALAR
    procedure ScalarMultiplication (Arr : in out MATRIX; Result: 
        in out MATRIX; Scalar : in out LONG_FLOAT) is

    begin
    -- BEGIN SPECIFIED OPERATION
        for Row in MATRIX'Range(1) loop
            for Column in MATRIX'Range(2) loop
                Result(Row, Column) := Arr(Row, Column) * Scalar;
            end loop;
        end loop;

    end ScalarMultiplication;

-- PROCEDURE FOR TRANSPOSE OF A MATRIX
    procedure Transpose (Arr : in out MATRIX; Result : in out MATRIX) is

    begin 
        for Column in MATRIX'Range(2) loop
            for Row in MATRIX'Range(1) loop
                Result(Row, Column) := Arr(Column, Row);
            end loop;
        end loop;

    end Transpose;

-- REFERENCE TO BASE DETERMINANT FUNCTION
    function Determinant (Arr : in out MATRIX; length : in INTEGER) 
        return LONG_FLOAT;

-- PROCEDURE TO FIND THE COFACTOR OF A SPECIFIED MATRIX
    procedure CoFactor (Arr : in out MATRIX; Arr2 : in out MATRIX; 
        p : in out INTEGER; q : in out INTEGER; length : in out INTEGER) is
    
    i : INTEGER;
    j : INTEGER;

    subMatrixSize : INTEGER;
    cofac : LONG_FLOAT;

    begin
    i := 1; j := 1;
    subMatrixSize := 6;

    -- FIND THE MINOR MATRIX OF THE MATRIX
    for row in 1 .. length loop
        for col in 1 .. length loop
            if row /= p and col /= q then
                Arr2(i,j) := Arr(row,col);
                j := j + 1;
                if j = length then
                    j := 1;
                    if i <= (length) then
                        i := i + 1;
                    end if;
                end if;
            end if;
        end loop;
    end loop;

    -- FIND THE DETERMINANT OF THE SUBMATRIX TO FIND THE COFACTOR AT INDEX
    cofac := Determinant (Arr2, subMatrixSize);
    cofac := ( (-1.0) **(coRow + coCol))*cofac;
    tmpMatrix4(coRow, coCol) := cofac;

    end CoFactor;

-- FUNCTION DETERMINANT REWRITTEN IN CONJUCTION W/ COFACTOR PROCEDURE 
    function Determinant2 (Arr : in out MATRIX; length : in INTEGER) 
        return LONG_FLOAT is
    
    det : LONG_FLOAT;
    subMatrix : MATRIX;
    startIndex : INTEGER;
    index : INTEGER;
    currDimension : INTEGER;
    sign : LONG_FLOAT;

    begin
        det := 0.0;
        startIndex := 1;
        currDimension := length;
        sign := 1.0;

        if length = 1 then 
            return Arr(1,1);
        end if;

        for x in 1 .. length loop
            index := x;
            -- Ada.Integer_Text_IO.Put(startIndex);
            CoFactor(Arr, subMatrix, startIndex, index, currDimension);
            det := det + (sign * Arr(1,x) * 
                Determinant2(subMatrix, (length - 1)));
            sign := (-1.0) * sign;
        end loop;

        return det;

    end Determinant2;

-- PROCEDURE FINDS THE ADJUGATE MATRIX OF A MATRIX FOR INVERSE CALCULATION
    procedure Adjugate (Arr : in out MATRIX; adj : in out MATRIX) is
    
    sign : LONG_FLOAT;
    tmpMatrix : MATRIX;
    subI : INTEGER;
    subJ : INTEGER;

    begin
        subI := 1;
        subJ := 1;
        sign := 1.0;
        for row in 1 .. 7 loop
            subI := row;
            for col in 1 .. 7 loop
                subJ := col;
                CoFactor (Arr, tmpMatrix,subI,subJ,Dimension);
                if ( ( (row+col) mod 2) = 0) then
                    sign := 1.0;
                else 
                    sign := -1.0;
                end if;
                adj(col, row) :=  (sign) * 
                    Determinant2(tmpMatrix, Dimension -1);
            end loop;
        end loop;

    end Adjugate;

-- PROCEDURE TO CALCULATE THE INVERSE OF A MATRIX
    procedure Inverse (Arr : in out MATRIX; inv : in out MATRIX) is
    determinant : LONG_FLOAT;
    adj : MATRIX; -- ADJUGATE Matrix

    begin
        determinant := Determinant2 (Arr, Dimension);
        Adjugate (Arr, adj);
        for i in 1 .. 7 loop
            for j in 1 .. 7 loop
                inv(i,j) := adj(i,j) / determinant;
            end loop;
        end loop;

    end inverse;

--  DETERMINANT FUNCTION WRITTEN TO WORK W/O RELYING ON COFACTOR PROCEDURE
    function Determinant (Arr : in out MATRIX; length : in INTEGER) 
        return LONG_FLOAT is
    
    subMatrix : MATRIX;
    subI : INTEGER := 0;
    subJ : INTEGER := 0;
    det : LONG_FLOAT;

    begin 
        det := 0.0;
        if length = 2 then
            return (  (Arr(1,1) * Arr(2,2))  -  (Arr(2,1) * Arr(1,2))  );
        
        else
            for x in 1 .. length loop
                subI := 1;
                for i in 2 .. length loop
                    subJ := 1;
                    for j in 1 .. length loop
                        if j /= x then 
                            subMatrix(subI, subJ) := Arr(i,j);
                            subJ := subJ + 1;
                        end if;
                    end loop;
                    subI := subI + 1;
                end loop;
                det := det +  ((Long_Float((-1)**x)) * Arr(1,x) * 
                    Determinant(subMatrix, length + (-1)));
            end loop;
        end if;
        return det;

    end Determinant;

-- BEGIN MAIN PROCEDURE
begin
    Start_Search (Search, Directory, Pattern);
    --SEARCHES EACH FILE IN CURRENT FOLDER
    while More_Entries (Search) loop
        Get_Next_Entry (Search, Dir_Ent);
        
        if Tail (Simple_Name (Dir_Ent), smatch'Length) = smatch then

            -- OPEN FILE FOR READING 
            -- PLACE THE CORRESPONDING FILES INTO A SPECIFIED MATRIX
            Open(inFile, In_File, Simple_Name (Dir_Ent));
            if Simple_Name (Dir_Ent) = "m1.in" then
                for Row in MATRIX'Range(1) loop
                    for Column in MATRIX'Range(2) loop
                        Ada.Integer_Text_IO.Get(inFile, inVal);
                        m1(Row, Column) := Long_Float(inVal);
                    end loop;
                end loop;
            
            elsif Simple_Name (Dir_Ent) = "m2.in" then
                for Row in MATRIX'Range(1) loop
                    for Column in MATRIX'Range(2) loop
                        Ada.Integer_Text_IO.Get(inFile, inVal);
                        m2(Row, Column) := Long_Float(inVal);
                    end loop;
                end loop;

            else 
                for Row in MATRIX'Range(1) loop
                    for Column in MATRIX'Range(2) loop
                        Ada.Integer_Text_IO.Get(inFile, inVal);
                        m3(Row, Column) := Long_Float(inVal);
                    end loop;
                end loop;

            end if;
            Close(inFile);

        end if;
    end loop;
   
    End_Search (Search);

-- BEGIN OPERATIONS ON MATRIX'S BY CALLING ON SPECIFIED PROCEDURES AND OR 
--      FUNCTIONS

    Put_Line("Operations");
    Put_Line("******************");

    Put_Line("m1 + m2");
    Addition(m1, m2, tmpMatrix1);
    PrintMatrix(tmpMatrix1);

    Put_Line("2*m2 + 3*m1- 5*m3");
    Scalar := 2.0; 
    ScalarMultiplication(m2, tmpMatrix2, Scalar);
    Scalar := 3.0;
    ScalarMultiplication(m1, tmpMatrix1, Scalar);
    Scalar := 5.0;
    ScalarMultiplication(m3, tmpMatrix3, Scalar);
    Addition(tmpMatrix2, tmpMatrix1, tmpMatrix4);
    Subtraction(tmpMatrix4, tmpMatrix3, tmpMatrix1);
    PrintMatrix(tmpMatrix1);

    Put_Line("(m2)^6");
    Multiplication(m2, m2, tmpMatrix1);
    Multiplication(tmpMatrix1, m2, tmpMatrix2);
    Multiplication(tmpMatrix2, m2, tmpMatrix3);
    Multiplication(tmpMatrix3, m2, tmpMatrix4);
    Multiplication(tmpMatrix4, m2, tmpMatrix1);
    PrintMatrix(tmpMatrix1);

    Put_Line("m1 * m2");
    Multiplication(m1, m2, tmpMatrix1);
    PrintMatrix(tmpMatrix1);

    Put_Line("m2 * m1");
    Multiplication(m2, m1, tmpMatrix2);
    PrintMatrix(tmpMatrix2);

    Put_Line("The transpose of m1 is:");
    Transpose(m1, tmpMatrix1);
    PrintMatrix(tmpMatrix1);

    Put_Line("The cofactor matrix of m1 is:");
    for i in 1 .. 7 loop
        coRow := i;
        for j in 1..7 loop
            coCol := j;
            CoFactor(m1, tmpMatrix1, coRow, coCol, Dimension);
        end loop;
    end loop;
    PrintMatrix(tmpMatrix4);
    

    Put_Line("The determinant matrix of m1 is:");
    d := Determinant2(m1, Dimension);
    Ada.Long_Float_Text_IO.Put(d, 10, 0, 0);
    Put_Line("");
    Put_Line("");

    Put_Line("The inverse matrix of m1 is:");
    Inverse(m1, inverseMatrix);
    PrintMatrix(inverseMatrix);

    Put_Line("m2 times its inverse m2^(-1_):");
    Inverse(m2, inverseMatrix);
    Multiplication(inverseMatrix, m2, tmpMatrix1);
    InversePrintMatrix(tmpMatrix1);

    Put_Line("The inverse matrix of m1*m2*m3 is:");
    Multiplication(m1, m2, tmpMatrix1);
    Multiplication(tmpMatrix1, m3, tmpMatrix2);
    Inverse(tmpMatrix2, inverseMatrix);
    PrintMatrix(inverseMatrix);

end matrixops7;