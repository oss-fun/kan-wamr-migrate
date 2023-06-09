use rand::Rng;

fn main() {
    const N: usize = 30;
    let mut map: [[[bool; N+2 as usize]; N+2]; 2] = [[[false; N+2 as usize]; N+2 as usize]; 2];

    let mut rng = rand::thread_rng();
    for i in 1..N+1 {
        for j in 1..N+1 {
            let r: i32 = rng.gen();
            map[0][i][j] = r % 2 == 0;
        }
    }

    let mut turn = 0;
    let mut k = 0;
    while turn < 101 {
        k += 1;
        println!("k: {}\n", k);

        // Display map
        for i in 1..=N as i32 {
            for j in 1..=N as i32 {
                print!("{}", if map[turn][i as usize][j as usize] {'#'} else {'.'});
            }
            println!("")
        }

        // next state
        for i in 1..N as i32+1 {
            for j in 1..N as i32+1 {

                // 周囲8方向のセル取得
                let mut num = 0;
                for _ni in i-1..=i+1 {
                    let mut ni: i32 = _ni as i32;
                    if ni == -1 {
                        ni = N as i32;
                    }
                    if ni == N as i32 +1 {
                        ni = 1;
                    }

                    for _nj in j-1..=j+1 {
                        let mut nj: i32 = _nj as i32;
                        if nj == -1 {
                            nj = N as i32;
                        }
                        if nj == N as i32 + 1 {
                            nj = 1;
                        }

                        if ni == i && nj == j {
                            continue
                        }

                        num += map[turn][ni as usize][nj as usize] as i32;
                    }
                }
                // let tmp = map[turn][if i+1 > N  {1} else {i+1}][j]
                //       + map[turn][if i-1 > 0 {i-1} else {N}][j]
                //       + map[turn][i][if j + 1 > N {1} else {j + 1}]
                //       + map[turn][i][if j - 1 > 0 {j-1} else {N}]
                //       + map[turn][if i+1 > N {1} else {i+1}][if j+1 > N {1} else {j+1}]
                //       + map[turn][i + 1 > N ? 1 : i + 1][j - 1 ?: N]
                //       + map[turn][i - 1 ?: N][j + 1 > N ? 1 : j + 1]
                //       + map[turn][i - 1 ?: N][j - 1 ?: N];

                if map[turn][i as usize][j as usize] {
                    map[turn ^ 1][i as usize][j as usize] = num == 2 || num == 3;
                }
                else {
                    map[turn ^ 1][i as usize][j as usize] = num == 3;
                }
            }
        }
        turn ^= 1;
    }
}
