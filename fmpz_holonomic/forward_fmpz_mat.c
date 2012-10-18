/*=============================================================================

    This file is part of ARB.

    ARB is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    ARB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ARB; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

=============================================================================*/
/******************************************************************************

    Copyright (C) 2012 Fredrik Johansson

******************************************************************************/

#include "fmpz_holonomic.h"

void
_fmpz_holonomic_eval_companion_matrix_fmpz(fmpz_mat_t M, fmpz_t Q,
    const fmpz_holonomic_t op, long n)
{
    fmpz_t c;
    long r = fmpz_holonomic_order(op);
    long i, j;

    fmpz_init(c);
    fmpz_set_si(c, n);

    fmpz_poly_evaluate_fmpz(Q, op->coeffs + r, c);

    for (i = 0; i < r - 1; i++)
    {
        for (j = 0; j < r; j++)
        {
            if (i + 1 == j)
                fmpz_set(M->rows[i] + j, Q);
            else
                fmpz_zero(M->rows[i] + j);
        }
    }

    for (j = 0; j < r; j++)
    {
        fmpz_poly_evaluate_fmpz(M->rows[r - 1] + j, op->coeffs + j, c);
        fmpz_neg(M->rows[r - 1] + j, M->rows[r - 1] + j);
    }

    fmpz_clear(c);
}


void
_fmpz_holonomic_forward_bsplit_fmpz(fmpz_mat_t M, fmpz_t Q,
    const fmpz_holonomic_t op, long start, long a, long b)
{
    long r = fmpz_holonomic_order(op);

    if (b - a == 1)
    {
        _fmpz_holonomic_eval_companion_matrix_fmpz(M, Q, op, start + a);
    }
    else
    {
        fmpz_mat_t L, R;
        fmpz_t Q2;
        long m = a + (b - a) / 2;

        fmpz_mat_init(L, r, r);
        fmpz_mat_init(R, r, r);
        fmpz_init(Q2);

        _fmpz_holonomic_forward_bsplit_fmpz(L, Q, op, start, a, m);
        _fmpz_holonomic_forward_bsplit_fmpz(R, Q2, op, start, m, b);

        fmpz_mat_mul(M, R, L);
        fmpz_mul(Q, Q, Q2);

        fmpz_mat_clear(L);
        fmpz_mat_clear(R);
        fmpz_clear(Q2);
    }
}

void
_fmpz_holonomic_bsplit_hypgeom_fmpz(fmpz_t P, fmpz_t Q,
    const fmpz_poly_t R0, const fmpz_poly_t R1, long start, long a, long b)
{
    if (b - a == 1)
    {
        fmpz c = start + a;
        fmpz_poly_evaluate_fmpz(P, R0, &c);
        fmpz_poly_evaluate_fmpz(Q, R1, &c);
        fmpz_neg(P, P);
    }
    else
    {
        fmpz_t P2, Q2;
        long m = a + (b - a) / 2;

        fmpz_init(P2);
        fmpz_init(Q2);

        _fmpz_holonomic_bsplit_hypgeom_fmpz(P, Q, R0, R1, start, a, m);
        _fmpz_holonomic_bsplit_hypgeom_fmpz(P2, Q2, R0, R1, start, m, b);
        fmpz_mul(P, P, P2);
        fmpz_mul(Q, Q, Q2);

        fmpz_clear(P2);
        fmpz_clear(Q2);
    }
}

void
fmpz_holonomic_forward_fmpz_mat(fmpz_mat_t M, fmpz_t Q,
    const fmpz_holonomic_t op, long start, long n)
{
    long r = fmpz_holonomic_order(op);

    if (r == 0 || n == 0)
    {
        fmpz_mat_one(M);
        fmpz_one(Q);
        return;
    }

    if (n < 0)
    {
        abort();
    }

    if (fmpz_holonomic_seq_is_cfinite(op))
    {
        _fmpz_holonomic_eval_companion_matrix_fmpz(M, Q, op, 0);
        fmpz_mat_pow(M, M, n);
        fmpz_pow_ui(Q, Q, n);
    }
    else if (fmpz_holonomic_seq_is_hypgeom(op))
    {
        _fmpz_holonomic_bsplit_hypgeom_fmpz(M->rows[0], Q,
            op->coeffs, op->coeffs + 1, start, 0, n);
    }
    else
    {
        _fmpz_holonomic_forward_bsplit_fmpz(M, Q, op, start, 0, n);
    }
}

