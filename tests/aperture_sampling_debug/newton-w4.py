

def newton_raphson():

  count = 1

  for x in range(0,100):
      # initial guess
      if count == 1:
          X = 0.1
          Y = 1

      # equations of the two circles
      f1 = X*X - 10*X + Y*Y - 10*Y + 34
      f2 = X*X - 22*X + Y*Y - 10*Y + 130

      if f1 == 0 and f2==0:
          break
      
      print "\r\nIteration=%d" %count
      count += 1
      
      print "f1= %f" %(f1)
      print "f2=%f" %(f2)
      
      # partial derivatives
      A = 2*X - 10
      B = 2*Y - 10
      C = 2*X - 22
      D = 2*Y - 10
      print "Partial derivatives:"
      print A,B
      print C,D

      J = [[A, B], [C, D]]

      invdetap =  1.0/(J[0][0]*J[1][1] - J[0][1]*J[1][0]);

      inv_jacobian =[
        [J[1][1] * invdetap, -J[0][1] * invdetap],
        [-J[1][0] * invdetap, J[0][0] * invdetap]]

      print "inv_jac", inv_jacobian

      dampen = 0.72

      X = X - dampen*(inv_jacobian[0][0]*f1 + inv_jacobian[0][1]*f2)
      Y = Y - dampen*(inv_jacobian[1][0]*f1 + inv_jacobian[1][1]*f2)

      print "X: ", X
      print "Y: ", Y



def w4():

  count = 1

  for x in range(0,100):
      # initial guess
      if count == 1:
          X = 0.1
          Y = 1

      # equations of the two circles
      f1 = X*X - 10*X + Y*Y - 10*Y + 34
      f2 = X*X - 22*X + Y*Y - 10*Y + 130

      if f1 == 0 and f2==0:
          break
      
      print "\r\nIteration=%d" %count
      count += 1
      
      print "f1= %f" %(f1)
      print "f2=%f" %(f2)
      
      # partial derivatives
      A = 2*X - 10
      B = 2*Y - 10
      C = 2*X - 22
      D = 2*Y - 10
      print "Partial derivatives:"
      print A,B
      print C,D

      J = [[A, B], 
           [C, D]]



      # decompose J into UL

      J_U = [[A, B],
             [0, D]]
      J_U_invdetap = 1.0/(J_U[0][0]*J_U[1][1] - J_U[0][1]*J_U[1][0]);
      J_U_inv = [J_U[1][1] * J_U_invdetap, -J_U[0][1] * J_U_invdetap],
                [-J_U[1][0] * J_U_invdetap, J_U[0][0] * J_U_invdetap]]

      J_L = [[A, 0],
             [C, D]]
      J_L_invdetap = 1.0/(J_L[0][0]*J_L[1][1] - J_L[0][1]*J_L[1][0]);
      J_L_inv = [J_L[1][1] * J_L_invdetap, -J_L[0][1] * J_L_invdetap],
                [-J_L[1][0] * J_L_invdetap, J_L[0][0] * J_L_invdetap]]



      invdetap =  1.0/(J[0][0]*J[1][1] - J[0][1]*J[1][0]);

      inv_jacobian =[
        [J[1][1] * invdetap, -J[0][1] * invdetap],
        [-J[1][0] * invdetap, J[0][0] * invdetap]]

      print "inv_jac", inv_jacobian


      X = X -(inv_jacobian[0][0]*f1 + inv_jacobian[0][1]*f2)
      Y = Y -(inv_jacobian[1][0]*f1 + inv_jacobian[1][1]*f2)

      print "X: ", X
      print "Y: ", Y




newton_raphson()